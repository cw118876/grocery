#ifndef BOOKMARK_SERVICE_TRANSFORM_HPP_
#define BOOKMARK_SERVICE_TRANSFORM_HPP_

#include <functional>
#include <type_traits>

#include "bookmark_service/actor.hpp"

namespace bms {

namespace detail {

template <typename Sender, typename Transformer>
class transform_impl {
 public:
  using in_type = typename Sender::out_type;
  using out_type = std::invoke_result_t<Transformer, in_type>;

  transform_impl(Sender&& sender, Transformer tran)
      : sender_{std::move(sender)}, tran_{tran} {}

  template <typename Handler>
  void set_out_handler(Handler&& h) {
    out_handler_ = std::forward<Handler>(h);
    sender_.set_out_handler(
        [this](out_type&& msg) { on_out_message(std::move(msg)); });
  }

  void on_out_message(out_type&& msg) {
    out_handler_(std::invoke(tran_, std::move(msg)));
  }

 private:
  Sender sender_;
  Transformer tran_;
  std::function<void(out_type&&)> out_handler_ = &noop_out_handler<out_type>;
};

template <typename Transform>
struct transform_helper {
  Transform tran_;
};

}  // namespace detail

template <typename Transform>
auto transform(Transform&& t) {
  return detail::transform_helper<Transform>{std::forward<Transform>(t)};
}

template <typename Sender, typename Transform>
auto make_transform(Sender&& sender, Transform&& f) {
  return detail::transform_impl<Sender, Transform>{std::forward<Sender>(sender),
                                                   std::forward<Transform>(f)};
}

template <typename Sender, typename Transform>
auto operator|(Sender&& sender, detail::transform_helper<Transform> h) {
  return make_transform(std::forward<Sender>(sender), h.tran_);
}

}  // namespace bms

#endif  // BOOKMARK_SERVICE_TRANSFORM_HPP_