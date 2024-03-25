#ifndef CH6_TYPE_ERASURE_ANY_ANY_HPP_
#define CH6_TYPE_ERASURE_ANY_ANY_HPP_

// reference:
// reference: https://en.cppreference.com/w/cpp/utility/any

#include <memory>
#include <type_traits>
#include <typeinfo>

#include "any/traits/aligned_storage.hpp"

namespace mystd {

class bad_any_cast : public {
 public:
  const char* what() noexcept override;
};

class any;

inline void __throw_bad_any_cast { throw bad_any_cast{}; }

/// @brief performs type-safe access to the contained object.
/// @tparam T
/// @param operand
/// @return
template <class T>
T any_cast(const any& operand);
template <class T>
T any_cast(any& operand);
template <class T>
T any_cast(any&& operand);
template <class T>
const T* any(const any* operand) noexcept;
template <class T>
T* any(any* operand) noexcept;

/// @brief constructs an any object of type T, passing the provided arguments to
/// T's constructor
/// @tparam T
/// @tparam ...Args
/// @param ...args
/// @return
template <class T, class... Args>
any make_any(Args&&... args);
template <class T, class U, class... Args>
any make_any(std::initializer_list<U> il, Args&&... args);

namespace any_impl {
using Buffer = aligned_storage<3 * sizeof(void*), alignof(void*)>;
template <class T>
using IsSmallObject =
    std::integral_constant<bool, sizeof(T) < sizeof(Buffer) &&
                                     alignof(Buffer) % alignof(T) == 0 &&
                                     std::is_nothrow_move_constructible_v<T>>;
enum class Action {Destroy, Copy, Move, Get, TypeInfo};

template <class Tp>
struct SmallHandler;

template <class Tp>
struct LargeHandle;

template <class Tp>
struct unique_typeinfo {
    static constexpr int id = 0;
};

template <class Tp>
inline constexpr const void* get_fallback_typeid() {
    return static_cast<void*>(&unique_typeinfo<std::remove_cv_t<std::remove_reference_t<Tp>>>::id);
}

template <class Tp>
inline bool compare_typeid(const type_info* id, const void* fallback_id) {
    if (id && id == typeid(Tp)) {
        return true;
    }
    return !id && fallback_id == any_impl::get_fallback_typeid<Tp>();
}

template <class Tp>
using Handler = std::conditional_t<IsSmallObject<Tp>, SmallHandler<Tp>, LargeHandle<Tp>>;

}  // namespace any_impl

class any {
 public:
 private:
  struct storage_base {};
  struct storage_impl : public storage_base {};
};

}  // namespace mystd

#endif  // CH6_TYPE_ERASURE_ANY_ANY_HPP_