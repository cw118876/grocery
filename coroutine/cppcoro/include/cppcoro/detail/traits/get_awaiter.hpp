#ifndef CPPCORO_DETAIL_TRAITS_GET_AWAITER_H_
#define CPPCORO_DETAIL_TRAITS_GET_AWAITER_H_

#include <type_traits>
#include <utility>

#include <cppcoro/detail/traits/any.hpp>
#include <cppcoro/detail/traits/is_awaiter.hpp>


namespace coro {
namespace detail {

// operator co_await is member function
// which is more preferred than non-member function
template <typename T>
auto get_awaiter_impl(T&& t, int) noexcept(
    noexcept(std::forward<T>(t).operator co_await()))
    -> decltype(std::forward<T>(t).operator co_await()) {
  return std::forward<T>(t).operator co_await();
}

// operator co_await is non-member function
template <typename T>
auto get_awaiter_impl(T&& t, long) noexcept(
    noexcept(operator co_await(std::forward<T>(t))))
    -> decltype(operator co_await(std::forward<T>(t))) {
  return operator co_await(std::forward<T>(t));
}

// T type is awaiter
template <typename T,
          std::enable_if_t<coro::detail::is_awaiter<T&&>::value, int> = 0>
auto get_awaiter_impl(T&& t, coro::detail::any) noexcept {
  return std::forward<T>(t);
}

template <typename T>
auto get_awaiter(T&& value) noexcept(
    noexcept(detail::get_awaiter_impl(std::forward<T>(value), 123)))
    -> decltye(detail::get_awaiter_impl(std::forward<T>(value), 123)) {
  return detail::get_awaiter_impl(std::forward<T>(value), 123);
}

}  // namespace detail
}  // namespace coro

#endif  // CPPCORO_DETAIL_TRAITS_GET_AWAITER_H_
