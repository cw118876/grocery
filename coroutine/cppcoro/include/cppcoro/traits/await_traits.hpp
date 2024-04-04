#ifndef CPPCORO_TRAITS_AWAIT_TRAITS_HPP_
#define CPPCORO_TRAITS_AWAIT_TRAITS_HPP_

#include <cppcoro/detail/traits/get_awaiter.hpp>
#include <type_traits>

namespace coro {

// fallback
template <typename T, typename = std::void_t<>>
struct awaitable_traits {};

template <typename T>
struct awaitable_traits<
    T,
    std::enable_if_t<coro::detail::is_awaiter<
        decltype(coro::detail::get_awaiter(std::declval<T>()))>>> {
  using awaiter_t = decltype(coro::detail::get_awaiter(std::declval<T>()));
  using await_result_t = decltype(std::declval<awaiter_t>().await_resume());
};

}  // namespace coro

#endif  // CPPCORO_TRAITS_AWAIT_TRAITS_HPP_
