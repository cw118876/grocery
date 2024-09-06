#ifndef EXECUTION_THREAD_HPP_
#define EXECUTION_THREAD_HPP_

#include "execution/util.hpp"

#include <condition_variable>
#include <mutex>
#include <thread>
#include <utility>

namespace ex {

struct run_loop : immovable {
  struct none {};
  struct task : immovable {
    task* next = nullptr;
    virtual void execute() {}
  };

  template <typename Receiver>
  struct operation : task {
    Receiver receiver_;
    run_loop& loop_;
    operation(run_loop& loop, Receiver rec) : receiver_{rec}, loop_{loop} {}
    friend void start(operation& self) { self.loop_.push_back(&self); }
    void execute() override { set_value(receiver_, none{}); }
  };
  struct sender {
    using result_type = none;
    run_loop& loop_;
    template <typename Receiver>
    friend operation<Receiver> connect(sender self, Receiver receiver) {
      return {self.loop_, receiver};
    }
  };
  struct scheduler {
    run_loop& loop_;
    friend sender schedule(scheduler& self) { return sender{self.loop_}; }
  };
  scheduler get_scheduler() { return {*this}; }

  void push_back(task* op) {
    std::unique_lock<std::mutex> lk{mtx_};
    op->next = &head_;
    tail_->next = op;
    tail_ = op;
    cv_.notify_one();
  }
  task* pop_front() {
    std::unique_lock<std::mutex> lk{mtx_};
    cv_.wait(lk, [&] { return head_.next != &head_ || finished_; });
    if (head_.next == &head_) {
      return nullptr;
    }
    return std::exchange(head_.next, head_.next->next);
  }
  void run() {
    while (auto* op = pop_front()) {
      op->execute();
    }
  }
  void finish() {
    std::unique_lock<std::mutex> lk{mtx_};
    finished_ = true;
    cv_.notify_all();
  }

 private:
  mutable std::mutex mtx_;
  std::condition_variable cv_;
  volatile bool finished_ = false;
  task head_;
  task* tail_{&head_};
};

class thread_context : private run_loop {
 public:
  using run_loop::finish;
  using run_loop::get_scheduler;
  void join() { th_.join(); }

 private:
  std::thread th_{[this] { run(); }};
};

}  // namespace ex

#endif  //  EXECUTION_THREAD_HPP_
