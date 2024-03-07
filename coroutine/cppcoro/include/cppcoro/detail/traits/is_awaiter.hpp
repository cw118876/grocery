#ifndef CPPCORO_DETAIL_TRAITS_IS_AWAITER_HPP_
#define CPPCORO_DETAIL_TRAITS_IS_AWAITER_HPP_

#include <coroutine>
#include <type_traits>

namespace coro {
namespace detail {

template <typename T>
using is_coroutine_handle = std::is_convertible<T, std::coroutine_handle<>>;

template <typename T>
inline constexpr bool is_coroutine_handle_v = is_coroutine_handle<T>::value;

// three difference return version for await_suspend:
// bool, void, coroutine
template <typename T>
struct is_valid_await_suspend_return_value
    : std::disjunction<std::is_void<T>, std::is_same<T, bool>,
                       is_coroutine_handle<T>> {};

template <typename T, typename = std::void_t<>>
struct has_awaiter_interface : std::false_type {};

template <typename T>
struct has_awaiter_interface<
    T, std::void_t<decltype(std::declval<T>().await_ready()),
                   decltype(std::declval<T>().await_suspend(
                       std::declval<std::coroutine_handle<void>>())),
                   decltype(std::declval<T>().await_resume())>>
    : std::true_type {};

template <typename T>
struct is_awaiter
    : std::conjunction<detail::has_awaiter_interface<T>,
                       std::is_constructible<
                           bool, decltype(std::declval<T>().await_ready())>,
                       detail::is_valid_await_suspend_return_value<
                           decltype(std::declval<T>().await_suspend(
                               std::declval<std::coroutine_handle<void>>()))>> {
};

}  // namespace detail
}  // namespace coro

#endif  // CPPCORO_DETAIL_TRAITS_IS_AWAITER_HPP_