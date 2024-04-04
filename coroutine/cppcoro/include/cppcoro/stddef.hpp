#ifndef CPPCORO_STDDEF_HPP_
#define CPPCORO_STDDEF_HPP_

#define CPPCORO_ASSUME(x) __attribute__((__assume__(x)))
#define CPPCORO_NOINLINE(x) __attribute__((noinline))
#define CPPCORO_INLINE __attribute__((always_inline))

#endif  // CPPCORO_STDDEF_HPP_
