#ifndef EXECUTION_EXECUTION_HPP_
#define EXECUTION_EXECUTION_HPP_
#include <condition_variable>
#include <exception>
#include <functional>
#include <iostream>
#include <mutex>
#include <type_traits>
#include <variant>

#include "execution/util.hpp"

namespace ex {

/// just utility

template <typename Receiver, typename Tp>
struct just_operation : immovable {
  Receiver receiver_;
  Tp value_;
  friend void start(just_operation& self) {
    set_value(self.receiver_, self.value_);
  }
};

template <typename Tp>
struct just_sender {
  Tp value_;
  using result_type = Tp;

  template <typename Receiver>
  friend just_operation<Receiver, Tp> connect(just_sender sender,
                                              Receiver receiver) {
    return {{}, receiver, sender.value_};
  }
};

template <typename Tp>
just_sender<Tp> just(Tp t) {
  return {t};
}

enum class operation_state { canceled = 0, stopped = 1, done = 2 };

// then utility
template <typename Receiver, typename Function>
struct then_receiver {
  Receiver receiver_;
  Function func_;

  template <typename... Args>
  friend void set_value(then_receiver& self, Args&&... args) {
    set_value(self.receiver_,
              std::invoke(self.func_, std::forward<Args>(args)...));
  }
  friend void set_error(then_receiver& self, Err error) {
    set_error(self.receiver_, error);
  }
  friend void set_done(then_receiver& self) { set_done(self.receiver_); }
};

template <typename Sender, typename Receiver, typename Function>
struct then_operation : immovable {
  operator_t<Sender, then_receiver<Receiver, Function>> op;
  friend void start(then_operation& self) { start(self.op); }
};

template <typename Sender, typename Function>
struct then_sender {
  Sender sender_;
  Function func_;
  using result_type = std::invoke_result_t<Function, sender_result_t<Sender>>;
  template <typename Receiver>
  friend then_operation<Sender, Receiver, Function> connect(then_sender self,
                                                            Receiver receiver) {
    return {{},
            connect(self.sender_,
                    then_receiver<Receiver, Function>{receiver, self.func_})};
  }
};

template <typename Sender, typename Function>
then_sender<Sender, Function> then(Sender s, Function f) {
  return {s, f};
}

/// sync_wait sender consumer

struct sync_wait_state {
  mutable std::mutex mtx_;
  std::condition_variable cv_;
  volatile bool done_ = false;
};

template <typename Tp>
using variant_result_t = std::variant<std::monostate, Tp, Err>;

template <typename Tp>
struct sync_receiver {
  sync_wait_state& state_;
  variant_result_t<Tp>& data_;

  template <typename... Args>
  friend void set_value(sync_receiver& self, Args&&... args) {
    std::unique_lock<std::mutex> lock{self.state_.mtx_};
    self.data_ = Tp{std::forward<Args>(args)...};
    self.state_.done_ = true;
    self.state_.cv_.notify_one();
  }

  friend void set_error(sync_receiver& self, Err error) {
    std::unique_lock<std::mutex> lock{self.state_.mtx_};
    self.data_ = error;
    self.state_.done_ = true;
    self.state_.cv_.notify_one();
  }
  friend void set_done(sync_receiver& self) {
    std::unique_lock<std::mutex> lock{self.state_.mtx_};
    self.state_.done_ = true;
    self.state_.cv_.notify_one();
  }
};

template <typename Sender>
variant_result_t<sender_result_t<Sender>> sync_wait(Sender s) {
  sync_wait_state state;
  using T = sender_result_t<Sender>;
  variant_result_t<T> data;
  auto op = connect(s, sync_receiver<T>{state, data});
  start(op);
  std::unique_lock<std::mutex> lock{state.mtx_};
  state.cv_.wait(lock, [&] { return state.done_; });
  return data;
}

struct cout_receiver {
  template <typename Args>
  friend void set_value(cout_receiver& self, Args&& args) {
    (void)self;
    std::cout << "Value: " << std::forward<Args>(args) << "\n";
  }

  friend void set_error(cout_receiver& self, Err error) {
    (void)self;
    try {
      if (error) {
        std::rethrow_exception(error);
      }
    } catch (const std::exception& e) {
      std::cout << "Caught exception: '" << e.what() << "'\n";
    }
    std::terminate();
  }
  friend void set_done(cout_receiver& self) { (void)self; }
};

}  // namespace ex

#endif  // EXECUTION_EXECUTION_HPP_