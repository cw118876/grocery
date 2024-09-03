#ifndef BOOKMARK_SERVICE_JOIN_HPP_
#define BOOKMARK_SERVICE_JOIN_HPP_

#include <functional>
#include <list>

#include "bookmark_service/actor.hpp"

namespace bms {

namespace detail {

template <typename Sender>
class join_impl {
 public:
  using out_type = typename Sender::out_type;
  explicit join_impl(Sender&& sender) : sender_(std::move(sender)) {}

  template <typename Handler>
  void set_out_handler(Handler h) {
    handler_ = h;
    sender_.set_out_handler([&](out_type&& msg) { on_out_message(msg); });
  }
  void on_out_message(out_type&& msg) {
    sources_.emplace_back(std::move(msg));
    sources_.back().set_out_handler(handler_);
  }

 private:
  Sender sender_;
  std::function<void(out_type&&)> handler_ {&noop_out_handler<out_type>};
  std::list<out_type> sources_;
};

struct join_helper {};

}  // namespace detail

template <typename Sender>
auto make_join(Sender&& sender) {
  return detail::join_impl<Sender>(std::forward<Sender>(sender));
}

inline auto join() {
  return detail::join_helper{};
}

template <typename Sender>
auto operator|(Sender&& sender, detail::join_helper) {
  return detail::join_impl<Sender>(std::forward<Sender>(sender));
}

}  // namespace bms

#endif  // BOOKMARK_SERVICE_JOIN_HPP_