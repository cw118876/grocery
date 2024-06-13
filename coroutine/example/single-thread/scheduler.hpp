#ifndef SCHEDULER_HPP_
#define SCHEDULER_HPP_

#include <coroutine>
#include <queue>
#include <stack>

struct Task {
  struct promise_type {
    std::suspend_always initial_suspend() noexcept { return {}; }
    std::suspend_always final_suspend() noexcept { return {}; }

    void unhandled_exception() { ; }
    void return_void() {}
    Task get_return_object() noexcept {
      return Task{std::coroutine_handle<promise_type>::from_promise(*this)};
    }
  };

  auto get_handle() noexcept -> std::coroutine_handle<promise_type> {
    return handle_;
  }
  std::coroutine_handle<promise_type> handle_;
};


class Scheduler {
public: 
  void add_task(std::coroutine_handle<> h)  {
    task_.push(h);
  }

  void run() {
    while (!task_.empty()) {
      auto t = task_.front();
      task_.pop();
      t.resume();
      if (t.done()) {
        t.destroy();
      } else {
        add_task(t);
      }
    }
  }

  auto suspend() -> std::suspend_always {
    return {};
  }
private:
  std::queue<std::coroutine_handle<>> task_;

};


#endif  // SCHEDULER_HPP_
