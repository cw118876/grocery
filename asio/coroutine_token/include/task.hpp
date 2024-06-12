#ifndef TASK_HPP_
#define TASK_HPP_

#include <coroutine>
#include <exception>
#include <iostream>

template <typename Tp = void>
struct Task;

namespace detail {

template <typename Tp>
struct PromiseBase {
  Task<Tp> get_return_object();
  auto initial_suspend() noexcept -> std::suspend_always { return {}; }

  //
  struct FinalAwaiter {
    bool await_ready() noexcept { return false; }

    template <typename Prom>
    auto await_suspend(std::coroutine_handle<Prom> coro) noexcept
        -> std::coroutine_handle<> {
      coro.promise().continuation_;
    }

    void await_resume() noexcept {}
  };

  auto final_suspend() noexcept -> FinalAwaiter { return {}; }

  void unhandled_exception() {
    try {
    } catch (const std::exception& e) {
      // just log some information and then rethrow
      std::cerr << "coroutine: caught exception: " << e.what() << "\n";
      throw;
    }
  }

  std::coroutine_handle<> continuation_ = std::noop_coroutine();
};

template <typename Tp>
struct Promise : public PromiseBase<Tp> {
  void return_value(Tp value) { value_ = value; }
  Tp await_resume() { return value_; }
  Tp value_;
};
template <>
struct Promise<void> : public PromiseBase<void> {
  void return_void() noexcept {}
  void await_resume() {}
};

}  // namespace detail

template <typename Tp>
struct Task {
  using promise_type = detail::Promise<Tp>;
  Task() : Task(nullptr) {}
  explicit Task(std::coroutine_handle<promise_type> h) : handle_{h} {}

  bool await_ready() noexcept { return false; }
  Tp await_resume() { return handle_.promise().await_resume(); }
  auto await_suspend(std::coroutine_handle<> h) -> std::coroutine_handle<> {
    handle_.promise().continuation_ = h;
    return handle_;
  }
  void resume() { handle_.resume(); }

  std::coroutine_handle<promise_type> handle_;
};

namespace detail {

template <typename Tp>
inline Task<Tp> PromiseBase<Tp>::get_return_object() {
  return Task<Tp>(
      std::coroutine_handle<typename Task<Tp>::promise_type>::from_promise(
          *this));
}

}  // namespace detail

#endif  // TASK_HPP_
