#ifndef BOOKMARK_SERVICE_ACTOR_HPP_
#define BOOKMARK_SERVICE_ACTOR_HPP_
#include <functional>
#include <type_traits>

namespace bms {

namespace detail {

template <typename Tp>
void noop_in_handler(Tp&&) {}

template <typename Tp>
void noop_out_handler(Tp&&) {}

template <typename T>
struct remove_cvref {
  using type = std::remove_cv_t<std::remove_reference_t<T>>;
};

template <typename T>
using remove_cvref_t = typename remove_cvref<T>::type;

template <typename InMsg, typename OutMsg>
class Actor {
  static_assert(std::is_same_v<InMsg, remove_cvref_t<InMsg>>,
                "InMsg doesn't support cv qualified or reference type");
  static_assert(std::is_same_v<OutMsg, remove_cvref_t<OutMsg>>,
                "OutMsg doesn't support cv qualified or reference type");

 public:
  using in_type = InMsg;
  using out_type = OutMsg;
  using in_handler_t = void(in_type&&);
  using out_handler_t = void(out_type&&);

  template <typename Tp>
  void set_in_handler(Tp&& t) {
    static_assert(std::is_constructible_v<std::function<in_handler_t>, Tp&&>,
                  "Tp should be able to construct input message handler");
    in_handler_ = std::forward<Tp>(t);
  }

  template <typename Tp>
  void set_out_handler(Tp&& t) {
    static_assert(std::is_constructible_v<std::function<out_handler_t>, Tp&&>,
                  "Tp should be able to construct out message handler");
    out_handler_ = std::forward<Tp>(t);
  }

  void on_in_message(InMsg&& msg) { in_handler_(std::move(msg)); }

  void on_out_message(OutMsg&& msg) { out_handler_(std::move(msg)); }

 protected:
  auto get_in_handler() -> std::function<in_handler_t> { return in_handler_; }
  auto get_out_handler() -> std::function<out_handler_t> {
    return out_handler_;
  }

 private:
  std::function<out_handler_t> out_handler_{&noop_out_handler<out_type>};
  std::function<in_handler_t> in_handler_{&noop_in_handler<in_type>};
};

template <typename InMsg>
class Actor<InMsg, void> {
  static_assert(std::is_same_v<InMsg, remove_cvref_t<InMsg>>,
                "InMsg doesn't support cv qualified or reference type");

 public:
  using in_type = InMsg;
  using in_handler_t = void(in_type&&);

  template <typename Tp>
  void set_in_handler(Tp&& t) {
    static_assert(std::is_constructible_v<std::function<in_handler_t>, Tp&&>,
                  "Tp should be able to construct input message handler");
    in_handler_ = std::forward<Tp>(t);
  }

  void on_in_message(InMsg&& msg) { in_handler_(std::move(msg)); }

 protected:
  auto get_in_handler() -> std::function<in_handler_t> { return in_handler_; }

 private:
  std::function<in_handler_t> in_handler_{&noop_in_handler<in_type>};
};

template <typename OutMsg>
class Actor<void, OutMsg> {
  static_assert(std::is_same_v<OutMsg, remove_cvref_t<OutMsg>>,
                "OutMsg doesn't support cv qualified or reference type");

 public:
  using out_type = OutMsg;
  using out_handler_t = void(out_type&&);

  template <typename Tp>
  void set_out_handler(Tp&& t) {
    static_assert(std::is_constructible_v<std::function<out_handler_t>, Tp&&>,
                  "Tp should be able to construct out message handler");
    out_handler_ = std::forward<Tp>(t);
  }

  void on_out_message(OutMsg&& msg) { out_handler_(std::move(msg)); }

 protected:
  auto get_out_handler() -> std::function<out_handler_t> {
    return out_handler_;
  }

 private:
  std::function<out_handler_t> out_handler_{&noop_out_handler<out_type>};
};

}  // namespace detail

template <typename InMsg, typename OutMsg>
using actor = detail::Actor<InMsg, OutMsg>;

template <typename InMsg>
using sink = detail::Actor<InMsg, void>;

template <typename OutMsg>
using source = detail::Actor<void, OutMsg>;

}  // namespace bms

#endif  // BOOKMARK_SERVICE_ACTOR_HPP_
