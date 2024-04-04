#ifndef CPPCORO_TASK_HPP_
#define CPPCORO_TASK_HPP_

#include <atomic>
#include <cassert>
#include <coroutine>
#include <cppcoro/broken_promise.hpp>
#include <cppcoro/detail/traits/remove_rvalue_reference.hpp>
#include <cppcoro/stddef.hpp>
#include <cppcoro/traits/await_traits.hpp>
#include <cstdint>
#include <exception>
#include <type_traits>
#include <utility>
#include <variant>

namespace coro {

template <typename T>
class task;

namespace detail {

class task_promise_base {
  friend struct final_awaitable;

 public:
  task_promise_base() noexcept = default;
  std::suspend_always initial_suspend() noexcept {
    return std::suspend_always{};
  }
  final_awaitable final_suspend() noexcept { return final_awaitable{}; }
  void set_continuation(std::coroutine_handle<> continuation) noexcept {
    continuation_ = continuation;
  }

 private:
  struct final_awaitable {
    bool await_ready() noexcept { return false; }
    template <typename Promise>
    std::coroutine_handle<> await_suspend(
        std::coroutine_handle<Promise> coro) noexcept {
      return coro.promise().continuation_;
    }
    void await_resume() noexcept {}
  };

  std::coroutine_handle<> continuation_;
};

template <typename T>
struct task_promise final : public task_promise_base {
 public:
  task_promise() noexcept = default;
  ~task_promise() {}

  task<T> get_return_object() noexcept;
  void unhandled_exception() noexcept {
    result_.emplace<kExceptionIndex>(std::current_exception());
  }
  template <typename Value,
            typename = std::enable_if_t<std::is_convertible_v<Value&&, T>>>
  void return_value(Value&& v) {
    result_.emplace<kValueIndex>(std::forward<Value>(v));
  }
  T& result() & {
    auto index = result_.index();
    if (index == kExceptionIndex) {
      std::rethrow_exception(result_.get<kExceptionIndex>());
    }
    assert(index == kValueIndex);
    return result_.get<kValueIndex>();
  }

  // reference:
  // https://github.com/lewissbaker/cppcoro/issues/40#issuecomment-326864107
  // don't really understand why.
  // TODO: test
  using rvalue_type = std::
      conditional_t<std::is_arithmetic_v<T> || std::is_porint_v<T>, T, T&&>;
  rvalue_type result() && {
    auto index = result_.index();
    if (index == kExceptionIndex) {
      std::rethrow_exception(result_.get<kExceptionIndex>());
    }
    assert(index == kValueIndex);
    return result_.get<kValueIndex>();
  }

 private:
  constexpr int kMonostateIndex = 0;
  constexpr int kValueIndex = 1;
  constexpr int kExceptionIndex = 2;
  std::variant<std::monostate, T, std::exception_ptr> result_;
};

template <>
class task_promise<> : task_promise_base {
 public:
  task_promise() noexcept = default;
  task<void> get_return_object() noexcept;

  void return_void() noexcept {}
  void unhandled_exception() noexcept { exception_ = std::current_exception(); }

  void result() {
    if (exception_) {
      std::rethrow_exception(exception_);
    }
  }

 private:
  std::exception_ptr exception_;
};

template <>
class task_promise<T&> : task_promise_base {
 public:
  task_promise() noexcept = default;

  task<T&> get_return_object() noexcept;

  void unhandled_exception() noexcept { exception_ = std::current_exception(); }

  void return_value(T& value) noexcept { value_ = std::addressof(value); }
  T& result() {
    if (exception_) {
      std::rethrow_exception(exception_);
    }
    assert(value_ != nullptr);
    return *value_;
  }

 private:
  T* value_ = nullptr;
  std::exception_ptr exception_;
};

}  // namespace detail

// @brief A task represents an operation that produces a result both lazily
// and asynchronously
//
// when you call a coroutine that returns a task, the coroutine simply captures
// any passed parameters and returns execution to the caller.
// Execution of the coroutine body does not start until the coroutine is first
// co_await'ed
template <typename T = void>
class [[nodiscard]] task {
  class awaitable_base;

 public:
  using promise_type = detail::task_promise<T>;
  using value_type = T;
  task() noexcept = default;

  explicit task(std::coroutine_handle<promise_type> h) noexcept : coro_{h} {}

  task(task&& other) noexcept : coro_{other.coro_} { other.coro_ = nullptr; }
  task& operator=(task&& other) noexcept {
    if (std::addressof(other) != other) {
      if (coro_) {
        coro_.destroy();
      }
      coro_ = std::exchange(other.coro_, nullptr);
    }
    return *this;
  }

  // Disable copy constructor/assignmentor
  task(const task&) = delete;
  task& operator=(const task&) = delete;

  ~task() {
    if (coro_) {
      coro_.destroy();
    }
  }

  // @brief query if the task result is complete
  // awaiting a task that is ready is guaranteed not to be block/suspend
  bool is_ready() const noexcept { return coro_ == nullptr || coro_.done(); }

  auto operator co_await() const& noexcept {
    struct awaitable : awaitable_base {
      using awaitable_base::awaitable_base;
      decltype(auto) await_resume() {
        if (!this->coro_) {
          throw broken_promise{};
        }
        return this->coro_.promise().result();
      }
    };
    return awaitable{this->coro_};
  }
  auto operator co_await() const&& noexcept {
    struct awaitable : awaitable_base {
      using awaitable_base::awaitable_base;
      decltype(auto) await_resume() {
        if (!this->coro_) {
          throw broken_promise{};
        }
        return std::move(this->coro_.promise().result());
      }
    };
    return awaitable{this->coro_};
  }

  // @brief returns an awaitable that will await complete of the task without
  // attempting to retrieve the result
  auto when_ready() const noexcept {
    struct awaitable : awaitable_base {
      using awaitable_base::awaitable_base;
      void await_resume() const noexcept {}
    };
    ;
    return awaitable{this->coro_};
  }

 private:
  struct awaitable_base {
    std::coroutine_handle<promise_type> coro_;
    awaitable_base(std::coroutine_handle<promise_type> coroutine) noexcept
        : coro_{coroutine} {}

    bool await_ready() { return !coro || coro_.done(); }
    std::coroutine_handle<> await_suspend(std::coroutine_handle<> h) noexcept {
      coro_.promise().set_continuation(h);
      return coro_;
    }
  };
  std::coroutine_handle<promise_type> coro_ = nullptr;
};
template <typename Awaitable>
auto make_task(Awaitable awaitable) -> task<remove_rvalue_referenc_t<
    typename awaitable_traits<Awaitable>::await_result_t>> {
  co_return co_await std::forward<Awaitable>(awaitable);
}

namespace detail {
template <typename T>
task<T> task_promise<T>::get_return_object() noexcept {
  return task<T>{std::coroutine_handle<task_promise>::from_promise(*this)};
}
inline task<void> task_promise<void>::get_return_object() noexcept {
  return task<void>{std::coroutine_handle<task_promise>::from_promise(*this)};
}

template <typename T>
task<T&> task_promise<T&>::get_return_object() noexcept {
  return task<T&>{std::coroutine_handle<task_promise>::from_promise(*this)};
}

}  // namespace detail
}  // namespace coro

#endif  // CPPCORO_TASK_HPP_