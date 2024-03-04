#ifndef CPPCORO_DETAIL_TRAITS_IS_AWAITER_H_
#define CPPCORO_DETAIL_TRAITS_IS_AWAITER_H_

#include <coroutine>
#include <type_traits>

namespace coro {
namespace detail {
template <typename T, typename = void>
struct is_coroutine_handle {
  static constexpr bool value = false;
};

template <typename T>
struct is_coroutine_handle<
    T, std::is_convertible_v<T, std::coroutine_handle<void>>> {
  static constexpr bool value = true;
};

// three difference return version for await_suspend:
// bool, void, coroutine
template <typename T>
struct is_valid_await_suspend_return_value
    : std::disjunction<std::is_void<T>, std::is_same<T, bool>,
                       is_coroutine_handle<T>> {};

template <typename T, typename void>
struct is_awaiter {
    static constexpr bool value = false;
}

template <typename T>
struct is_awaiter<T, std::void_t<
   decltype(std::declval<T>().await_ready()),
   decltype(std::declval<T>().await_suspend(std::declval<std::coroutine_handle<void>>())),
   decltype(std::declval<T>().await_resume()),
   
  >>{
    static constexpr bool value = false;
}

}  // namespace detail
}  // namespace coro

#endif  // CPPCORO_DETAIL_TRAITS_IS_AWAITER_H_