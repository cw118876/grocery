#ifndef SCHEDULER_HPP_
#define SCHEDULER_HPP_

#include <atomic>
#include <condition_variable>
#include <coroutine>
#include <mutex>
#include <queue>
#include <stack>
#include <system_error>

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
  Scheduler(size_t num = std::thread::hardware_concurrency());
  void add_task(std::coroutine_handle<> h);
  void wait();
  void stop();
  void schedule();
  auto suspend() -> std::suspend_always { return {}; }

 private:
  void process(std::coroutine_handle<> h);
  std::queue<std::coroutine_handle<>> tasks_;
  std::vector<std::coroutine_handle<>> commitedTask_;
  std::vector<std::thread> workers_;
  std::mutex mu_;
  std::condition_variable cv_;
  bool stopped_ = {false};
  std::atomic<size_t> finished_{0};
};

Scheduler::Scheduler(size_t num) {
  workers_.reserve(num);
  for (size_t idx = 0; idx < num; ++idx) {
    workers_.emplace_back([this]() {
      while (true) {
        std::coroutine_handle<> t;
        {
          std::unique_lock lock{mu_};
          cv_.wait(lock, [this] { return stopped_ || (!tasks_.empty()); });
          if (stopped_) {
            return;
          }
          t = tasks_.front();
          tasks_.pop();
        }
        if (t) {
          process(t);
        }
      }
    });
  }
}

void Scheduler::process(std::coroutine_handle<> h) {
  h.resume();
  if (!h.done()) {
    std::unique_lock lock{mu_};
    tasks_.push(h);
    cv_.notify_one();
  } else {
    h.destroy();
    if (finished_.fetch_add(1) + 1 == commitedTask_.size()) {
      {
        std::unique_lock lock{mu_};
        stopped_ = true;
      }

      cv_.notify_all();
    }
  }
}

void Scheduler::add_task(std::coroutine_handle<> h) {
  std::unique_lock lock{mu_};
  commitedTask_.push_back(h);
}

void Scheduler::schedule() {
  {
    std::unique_lock lock{mu_};
    for (auto t : commitedTask_) {
      tasks_.push(t);
    }
  }
  cv_.notify_all();
}

void Scheduler::wait() {
  for (auto& th : workers_) {
    th.join();
  }
}

void Scheduler::stop() {
  {
    std::unique_lock lock{mu_};
    stopped_ = true;
  }

  cv_.notify_all();
  wait();
}

#endif  // SCHEDULER_HPP_
