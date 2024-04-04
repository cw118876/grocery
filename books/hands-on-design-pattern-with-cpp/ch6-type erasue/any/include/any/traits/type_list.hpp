#ifndef CH6_TYPE_ERASURE_ANY_TRAITS_TYPE_LIST_HPP_
#define CH6_TYPE_ERASURE_ANY_TRAITS_TYPE_LIST_HPP_

#include <cstddef>

namespace mystd {

template <class Hp, class Tp>
struct type_list {
  using Head = Hp;
  using Tail = Tp;
};

/// @brief  find the first one TypeList such that size <=  TypeList::Head
/// @tparam TypeList
/// @tparam Size
/// @tparam
template <class TypeList,
          size_t Size,
          bool = Size <= sizeof(typename TypeList::Head)>
struct find_first;

template <class Hp, class Tp, size_t Size>
struct find_first<type_list<Hp, Tp>, Size, true> {
  using type = Hp;
};

template <class Hp, class Tp, size_t Size>
struct find_first<type_list<Hp, Tp>, Size, false> {
  using type = typename find_first<Tp, Size>::type;
};

}  // namespace mystd

#endif  // CH6_TYPE_ERASURE_ANY_TRAITS_TYPE_LIST_HPP_
