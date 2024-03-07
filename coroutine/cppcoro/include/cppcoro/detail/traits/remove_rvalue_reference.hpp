#ifndef CPPCORO_DETAIL_TRAITS_REMOVE_RVALUE_REFERENCE_HPP_
#define CPPCORO_DETAIL_TRAITS_REMOVE_RVALUE_REFERENCE_HPP_

namespace coro {
template <typename T>
struct remove_rvalue_referenc {
    using type = T;
};

template <typename T>
struct remove_rvalue_referenc<T&&> {
    using type = T;
};

template <typename T>
using remove_rvalue_referenc_t = typename remove_rvalue_referenc<T>::value;
}  // namespace coro


#endif  // CPPCORO_DETAIL_TRAITS_REMOVE_RVALUE_REFERENCE_HPP_
