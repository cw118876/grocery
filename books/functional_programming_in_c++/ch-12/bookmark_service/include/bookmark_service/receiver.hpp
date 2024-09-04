#ifndef BOOKMARK_SERVICE_RECEIVER_HPP_
#define BOOKMARK_SERVICE_RECEIVER_HPP_

#include <functional>
#include <type_traits>

namespace bms {

namespace detail {

template <typename Sender,
          typename Handler,
          typename MsgType = typename Sender::out_type>
class receiver_impl {
 public:
  receiver_impl(Sender&& sender, Handler h)
      : sender_(std::move(sender)), handler_(h) {
    sender_.set_out_handler(
        [this](MsgType&& msg) { on_in_message(std::move(msg)); });
  }

  void on_in_message(MsgType&& msg) { std::invoke(handler_, std::move(msg)); }

 private:
  Sender sender_;
  Handler handler_;
};

template <typename Handler>
struct receiver_helper {
  Handler handler_;
};

}  // namespace detail

template <typename Handler>
auto receiver(Handler&& h) {
  return detail::receiver_helper<Handler>{std::forward<Handler>(h)};
}

template <typename Sender, typename Handler>
auto make_receiver(Sender&& sender, Handler&& h) {
  return detail::receiver_impl<Sender, Handler>{std::forward<Sender>(sender),
                                                std::forward<Handler>(h)};
}

template <typename Sender, typename Handler>
auto operator|(Sender&& sender, detail::receiver_helper<Handler> h) {
  return make_receiver(std::forward<Sender>(sender), h.handler_);
}

}  // namespace bms

#endif  // BOOKMARK_SERVICE_RECEIVER_HPP_