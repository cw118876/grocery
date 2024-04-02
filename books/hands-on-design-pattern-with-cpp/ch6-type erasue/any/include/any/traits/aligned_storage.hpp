#ifndef CH6_TYPE_ERASURE_ANY_TRAITS_ALIGNED_STORAGE_HPP_
#define CH6_TYPE_ERASURE_ANY_TRAITS_ALIGNED_STORAGE_HPP_

#include <cstddef>
#include <type_traits>

#include "any/traits/nat.hpp"
#include "any/traits/type_list.hpp"

namespace mystd {

template <class T>
struct aligned_type {
  static constexpr size_t value = alignof(T);
  using type = T;
};

struct struct_double {
  long double lx_;
};

struct struct_double4 {
  long double lx_[4];
};

using all_types = type_list<
    aligned_type<unsigned char>,
    type_list<
        aligned_type<unsigned short>,
        type_list<
            aligned_type<unsigned int>,
            type_list<
                aligned_type<unsigned long>,
                type_list<
                    aligned_type<unsigned long long>,
                    type_list<
                        aligned_type<double>,
                        type_list<
                            aligned_type<long double>,
                            type_list<aligned_type<struct_double>,
                                      type_list<aligned_type<struct_double4>,
                                                type_list<aligned_type<char*>,
                                                          nat>>>>>>>>>>;

template <size_t Align>
struct alignas(Align) fallback_overaligned {};

template <class TL, size_t Align>
struct find_pod;

template <class Hp, size_t Align>
struct find_pod<type_list<Hp, nat>, Align> {
  using type = std::conditional_t<Align == Hp::value, typename Hp::type,
                                  fallback_overaligned<Align>>;
};

template <class Hp, class Tp, size_t Align>
struct find_pod<type_list<Hp, Tp>, Align> {
  using type = std::conditional_t<Align == Hp::value, typename Hp::type,
                                  typename find_pod<Tp, Align>::type>;
};

template <class TL, size_t Len>
struct find_max_align;

template <class Hp, size_t Len>
struct find_max_align<type_list<Hp, nat>, Len>
    : public std::integral_constant<size_t, Hp::value> {};

template <size_t Len, size_t A1, size_t A2>
struct select_align {
 private:
  static constexpr size_t min_ = A1 < A2 ? A1 : A2;
  static constexpr size_t max_ = A1 < A2 ? A2 : A1;

 public:
  static constexpr size_t value = Len < max_ ? min_ : max_;
};

template <class Hp, class Tp, size_t Size>
struct find_max_align<type_list<Hp, Tp>, Size>
    : public std::integral_constant<
          size_t, select_align<Size, Hp::value,
                               find_max_align<Tp, Size>::value>::value> {};

template <size_t Size, size_t Align = find_max_align<all_types, Size>::value>
struct aligned_storage {
  using Aligner = typename find_pod<all_types, Align>::type;
  union type {
    Aligner align_;
    unsigned char data_[(Size + Align - 1) / Align * Align];
  };
};

template <size_t Size, size_t Align = find_max_align<all_types, Size>::value>
using aligned_storage_t = typename aligned_storage<Size, Align>::type;

#define CREATE_ALIGNED_STORAGE_SPECIALIZATION(n) \
  template <size_t Size>                         \
  struct aligned_storage<Size, n> {              \
    struct alignas(n) type {                     \
      unsigned char lx_[(Size + n - 1) / n * n]; \
    };                                           \
  }
CREATE_ALIGNED_STORAGE_SPECIALIZATION(0x1);
CREATE_ALIGNED_STORAGE_SPECIALIZATION(0x2);
CREATE_ALIGNED_STORAGE_SPECIALIZATION(0x4);
CREATE_ALIGNED_STORAGE_SPECIALIZATION(0x8);
CREATE_ALIGNED_STORAGE_SPECIALIZATION(0x10);
CREATE_ALIGNED_STORAGE_SPECIALIZATION(0x20);
CREATE_ALIGNED_STORAGE_SPECIALIZATION(0x40);
CREATE_ALIGNED_STORAGE_SPECIALIZATION(0x80);
CREATE_ALIGNED_STORAGE_SPECIALIZATION(0x100);
CREATE_ALIGNED_STORAGE_SPECIALIZATION(0x200);
CREATE_ALIGNED_STORAGE_SPECIALIZATION(0x400);
CREATE_ALIGNED_STORAGE_SPECIALIZATION(0x800);
CREATE_ALIGNED_STORAGE_SPECIALIZATION(0x1000);
CREATE_ALIGNED_STORAGE_SPECIALIZATION(0x2000);
CREATE_ALIGNED_STORAGE_SPECIALIZATION(0x4000);

}  // namespace mystd

#endif  // CH6_TYPE_ERASURE_ANY_TRAITS_ALIGNED_STORAGE_HPP_
