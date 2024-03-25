#ifndef CH6_TYPE_ERASURE_ANY_TRAITS_NAT_HPP_
#define CH6_TYPE_ERASURE_ANY_TRAITS_NAT_HPP_

namespace mystd {
struct nat {
  nat() = delete;
  nat(const nat&) = delete;
  nat& operator=(const nat&) = delete;
  ~nat() = delete;
};
}  // namespace mystd

#endif  // CH6_TYPE_ERASURE_ANY_TRAITS_NAT_HPP_
