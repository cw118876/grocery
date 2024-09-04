#ifndef BOOKMARK_SERVICE_FILTER_HPP_
#define BOOKMARK_SERVICE_FILTER_HPP_

#include <functional>

#include "bookmark_service/actor.hpp"

namespace bms {

namespace detail {

template <typename Sender, typename Predictor>
class filter_impl {
 public:
  using out_type = typename Sender::out_type;
  filter_impl(Sender&& sender, Predictor pred) : sender_{std::move(sender)}, pred_(pred) {}

  template <typename Handler>
  void set_out_handler(Handler h) {
    handler_ = h;
    sender_.set_out_handler(
        [&](out_type&& msg) { on_out_message(std::move(msg)); });
  }

  void on_out_message(out_type&& msg) {
    if (std::invoke(pred_, msg)) {
      handler_(std::move(msg));
    }
  }

 private:
  Sender sender_;
  Predictor pred_;
  std::function<void(out_type&&)> handler_{&noop_out_handler<out_type>};
};

template <typename Predictor>
struct filter_helper {
  Predictor pred_;
};

}  // namespace detail

template <typename Predictor>
auto filter(Predictor&& pred) {
  return detail::filter_helper<Predictor>{std::forward<Predictor>(pred)};
}

template <typename Sender, typename Predictor>
auto make_filter(Sender&& sender, Predictor&& pred) {
  return detail::filter_impl<Sender, Predictor>(std::forward<Sender>(sender),
                                                std::forward<Predictor>(pred));
}

template <typename Sender, typename Predictor>
auto operator|(Sender&& sender, detail::filter_helper<Predictor> pred) {
  return make_filter(std::forward<Sender>(sender), pred.pred_);
}

}  // namespace bms

#endif  // BOOKMARK_SERVICE_FILTER_HPP_
