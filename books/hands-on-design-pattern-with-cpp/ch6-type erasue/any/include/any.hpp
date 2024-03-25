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
enum class Action { Destroy, Copy, Move, Get, TypeInfo };

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
  return static_cast<void*>(
      &unique_typeinfo<std::remove_cv_t<std::remove_reference_t<Tp>>>::id);
}

template <class Tp>
inline bool compare_typeid(const type_info* id, const void* fallback_id) {
  if (id && id == typeid(Tp)) {
    return true;
  }
  return !id && fallback_id == any_impl::get_fallback_typeid<Tp>();
}

template <class Tp>
using Handler =
    std::conditional_t<IsSmallObject<Tp>, SmallHandler<Tp>, LargeHandle<Tp>>;

}  // namespace any_impl

class any {
 public:
  constexpr any() noexcept : h_{nullptr} {}

  any(const any& other) : h_{nullptr} {
    if (other.h_) {
      other.call(Action::Copy, this);
    }
  }
  any(any&& other) : h_{nullptr} {
    if (other.h_) {
      other.call(Action::Move, this);
    }
  }

  template <class ValueType, class Tp = std::decay_t<ValueType>,
            class = std::enable_if_t<!std::is_same_v<Tp, any> &&
                                     !std::__is_in_place_type_v<ValueType> &&
                                     std::is_copy_constructible_v<Tp>>>
  any(ValueType&& value);

  template <class ValueType, class... Args, class Tp = std::decay_t<ValueType>,
            class = std::enable_if_t<std::is_constructible_v<Tp, Args...> &&
                                     std::is_copy_constructible_v<Tp>>>
  any(std::in_place_type<ValueType>, Args&&... args);

  template <
      class ValueType, class U, class... Args,
      class Tp = std::decay_t<ValueType>,
      class = std::enable_if_t<
          std::is_constructible_v<Tp, std::initializer_list<U>, Args...> &&
          std::is_copy_constructible_v<Tp>>>
  any(std::in_place_type<ValueType>, std::initializer_list<U> u,
      Args&&... args);
  ~any() {this->reset();}

  any& operator=(const any& other) {
    any(other).swap(*this);
    return *this;
  }
  any& operator=(any&& other) noexcept {
    any(std::move(other)).swap(*this);
    return *this;
  }

  template <class ValueType,
            class Tp = std::decay_t<ValueType>
            class = std::enable_if_t<!std::is_same_v<Tp, any> && std::is_copy_constructible_v<Tp>>>
  any& operator=(ValueType&& rhs);

  template <class ValueType,
            class... Args,
            class Tp = std::decay_t<ValueType>,
            class = std::enable_if_t<std::is_copy_constructible_v<Tp, Args...> && std::is_copy_constructible_v<Tp>>>
  Tp& emplace(Args&&... args);
  
  template <class ValueType,
            class U,
            class... Args,
            class Tp = std::decay_t<ValueType>,
            class = std::enable_if_t<std::is_copy_constructible_v<Tp, std::initializer_list<U>, Args...> &&
                                     std::is_copy_constructible_v<Tp>>>
  Tp& emplace(std::initializer_list<U> u, Args&&... args);


  void reset() noexcept {
    if (h_) {
        call(Action::Destroy);
    }
  }
  void swap(any& other) noexcept;
  bool has_value() const noexcept {return h_ == nullptr;}
  const type_info& type() const noexcept {
    if (h_) {
        return *static_cast<const type_info*>(this->call(Action::TypeInfo));
    } else {
        return typeid(void);
    }
  }

 private:
  using Action = any_impl::Action;
  using HandlerFuncPtr = void* (*)(Action, const any*, any*, const type_info*,
                                   const void* fallback_info = nullptr);
  union Storage {
    constexpr Storage() : ptr_{nullptr} {}
    void* ptr_;
    any_impl::Buffer buf_;
  };
  void call(Action a, any* other = nullptr, const type_info* info = nullptr,
            const void fallback_info = nullptr) {
    return h_(a, this, other, info, fallback_info);
  }
  void call(Action a, any* other = nullptr, const type_info* info = nullptr,
            const void fallback_info = nullptr) const {
    return h_(a, this, other, info, fallback_info);
  }
  template <class>
  friend struct any_impl::SmallHandler;
  template <class>
  friend struct any_impl::LargeHandle;

  template <class T>
  friend const T* any(const any* operand) noexcept;
  template <class T>
  friend T* any(any* operand) noexcept;
  Storage s_;
  HandlerFuncPtr h_ = nullptr;
};


namespace any_impl {


}  // namespace any_impl


}  // namespace mystd

#endif  // CH6_TYPE_ERASURE_ANY_ANY_HPP_