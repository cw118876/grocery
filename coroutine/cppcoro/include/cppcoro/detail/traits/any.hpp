#ifndef CPPCORO_DETAIL_TRAITS_ANY_H_
#define CPPCORO_DETAIL_TRAITS_ANY_H_

namespace coro {
namespace detail {
/// @brief  helper type that can be cast-to from any type
struct any {
  template <typename T>
  any(T &&) noexcept {}
};

}  // namespace detail
}  // namespace coro

#endif  // CPPCORO_DETAIL_TRAITS_ANY_H_
