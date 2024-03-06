#ifndef CPPCORO_TASK_H_
#define CPPCORO_TASK_H_

#include <atomic>
#include <cassert>
#include <coroutine>
#include <cppcoro/broken_promise.hpp>
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

  // reference: https://github.com/lewissbaker/cppcoro/issues/40#issuecomment-326864107
  // don't really understand why.
  // TODO: test 
  using rvalue_type =
      std::conditional_t<std::is_arithmetic_v<T> || std::is_porint_v<T>, T,
                         T&&>;
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
class task_promise<>: task_promise_base {
 public:
 task_promise() noexcept = default;
 task<void> get_return_object() noexcept;

 void return_void() noexcept {}
 void unhandled_exception() noexcept {
    exception_ = std::current_exception();
 }

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

  void unhandled_exception() noexcept {
    exception_ = std::current_exception();
  }

  void return_value(T& value) noexcept {
    value_ = std::addressof(value);
  }
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

}  // namespace coro

#endif  // CPPCORO_TASK_H_