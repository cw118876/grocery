#ifndef CH6_TYPE_ERASURE_ANY_ANY_HPP_
#define CH6_TYPE_ERASURE_ANY_ANY_HPP_

// reference:
// reference: https://en.cppreference.com/w/cpp/utility/any

#include <initializer_list>
#include <memory>
#include <type_traits>
#include <typeinfo>
#include <utility>

#include "any/traits/aligned_storage.hpp"

namespace mystd {

class bad_any_cast : public std::bad_cast {
 public:
  const char* what() const noexcept override;
};

class any;

inline void throw_bad_any_cast() { throw bad_any_cast{}; }

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
const T* any_cast(const any* operand) noexcept;
template <class T>
T* any_cast(any* operand) noexcept;

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
  return static_cast<const void*>(
      &unique_typeinfo<std::remove_cv_t<std::remove_reference_t<Tp>>>::id);
}

template <class Tp>
inline bool compare_typeid(const std::type_info* id, const void* fallback_id) {
  if (id && *id == typeid(Tp)) {
    return true;
  }
  return !id && fallback_id == any_impl::get_fallback_typeid<Tp>();
}

template <class Tp>
using Handler = std::conditional_t<IsSmallObject<Tp>::value, SmallHandler<Tp>,
                                   LargeHandle<Tp>>;

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
  any(std::in_place_type_t<ValueType>, Args&&... args);

  template <
      class ValueType, class U, class... Args,
      class Tp = std::decay_t<ValueType>,
      class = std::enable_if_t<
          std::is_constructible_v<Tp, std::initializer_list<U>, Args...> &&
          std::is_copy_constructible_v<Tp>>>
  any(std::in_place_type_t<ValueType>, std::initializer_list<U> u,
      Args&&... args);
  ~any() { this->reset(); }

  any& operator=(const any& other) {
    any(other).swap(*this);
    return *this;
  }
  any& operator=(any&& other) noexcept {
    any(std::move(other)).swap(*this);
    return *this;
  }

  template <class ValueType, class Tp = std::decay_t<ValueType>,
            class = std::enable_if_t<!std::is_same_v<Tp, any> &&
                                     std::is_copy_constructible_v<Tp>>>
  any& operator=(ValueType&& rhs);

  template <
      class ValueType, class... Args, class Tp = std::decay_t<ValueType>,
      class = std::enable_if_t<std::is_copy_constructible_v<Tp, Args...> &&
                               std::is_copy_constructible_v<Tp>>>
  Tp& emplace(Args&&... args);

  template <
      class ValueType, class U, class... Args,
      class Tp = std::decay_t<ValueType>,
      class = std::enable_if_t<
          std::is_copy_constructible_v<Tp, std::initializer_list<U>, Args...> &&
          std::is_copy_constructible_v<Tp>>>
  Tp& emplace(std::initializer_list<U> u, Args&&... args);

  void reset() noexcept {
    if (h_) {
      call(Action::Destroy);
    }
  }
  void swap(any& other) noexcept;
  bool has_value() const noexcept { return h_ != nullptr; }
  const std::type_info& type() const noexcept {
    if (h_) {
      return *static_cast<const std::type_info*>(this->call(Action::TypeInfo));
    } else {
      return typeid(void);
    }
  }

 private:
  using Action = any_impl::Action;
  using HandlerFuncPtr = void* (*)(Action, const any*, any*,
                                   const std::type_info*,
                                   const void* fallback_info);
  union Storage {
    constexpr Storage() : ptr_{nullptr} {}
    void* ptr_;
    any_impl::Buffer buf_;
  };
  void* call(Action a, any* other = nullptr,
             const std::type_info* info = nullptr,
             const void* fallback_info = nullptr) {
    return h_(a, this, other, info, fallback_info);
  }
  void* call(Action a, any* other = nullptr,
             const std::type_info* info = nullptr,
             const void* fallback_info = nullptr) const {
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
  template <class T>
  friend T any_cast(const any& operand);
  template <class T>
  friend T any_cast(any& operand);
  template <class T>
  friend T any_cast(any&& operand);
  template <class T>
  const T* any_cast(const any* operand) noexcept;
  template <class T>
  friend T* any_cast(any* operand) noexcept;
  Storage s_;
  HandlerFuncPtr h_ = nullptr;
};

namespace any_impl {

template <class Tp>
struct SmallHandler {
  using Alloc = std::allocator<Tp>;
  using ATraits = std::allocator_traits<Alloc>;

  static void* handle(Action act, const any* self, any* other,
                      const std::type_info* info, const void* fallback_info) {
    switch (act) {
      case Action::Destroy:
        destroy(const_cast<any&>(*self));
        return nullptr;
      case Action::Copy:
        copy(*self, *other);
        return nullptr;
      case Action::Move:
        move(const_cast<any&>(*self), *other);
        return nullptr;
      case Action::Get:
        return get(const_cast<any&>(*self), info, fallback_info);
      case Action::TypeInfo:
        return typeinfo();
    }
    __builtin_unreachable();
  }

  template <class... Args>
  static Tp& create(any& dst, Args&&... args) {
    Alloc a;
    Tp* p = static_cast<Tp*>(static_cast<void*>(std::addressof(dst.s_.buf_)));
    ATraits::construct(a, p, std::forward<Args>(args)...);
    dst.h_ = &SmallHandler::handle;
    return *p;
  }
  static void destroy(any& self) {
    Alloc a;
    Tp* p = static_cast<Tp*>(static_cast<void*>(std::addressof(self.s_.buf_)));
    ATraits::destroy(a, p);
    self.h_ = nullptr;
  }

  static void copy(const any& self, any& dst) {
    SmallHandler::create(
        dst, *static_cast<const Tp*>(static_cast<const void*>(&self.s_.buf_)));
  }
  static void move(any& self, any& dst) {
    SmallHandler::create(
        dst, std::move(*static_cast<Tp*>(static_cast<void*>(&self.s_.buf_))));
  }
  static void* get(any& self, const std::type_info* info,
                   const void* fallback_id) {
    if (any_impl::compare_typeid<Tp>(info, fallback_id)) {
      return static_cast<void*>(&self.s_.buf_);
    }
    return nullptr;
  }
  static void* typeinfo() {
    return const_cast<void*>(static_cast<const void*>(&typeid(Tp)));
  }
};

template <class Tp>
struct LargeHandle {
  using Alloc = std::allocator<Tp>;
  using ATraits = std::allocator_traits<Alloc>;
  static void* handle(Action act, const any* self, any* other,
                      const std::type_info* info, const void* fallback_info) {
    switch (act) {
      case Action::Destroy:
        destroy(const_cast<any&>(*self));
        return nullptr;
      case Action::Copy:
        copy(*self, *other);
        return nullptr;
      case Action::Move:
        move(const_cast<any&>(*self), *other);
        return nullptr;
      case Action::Get:
        return get(const_cast<any&>(*self), info, fallback_info);
      case Action::TypeInfo:
        return typeinfo();
    }
    __builtin_unreachable();
  }
  template <class... Args>
  static Tp& create(any& dst, Args&&... args) {
    Alloc a;
    Tp* p = ATraits::allocate(a, sizeof(Tp));
    ATraits::construct(a, p, std::forward<Args>(args)...);
    dst.h_ = &LargeHandle::handle;
    dst.s_.ptr_ = static_cast<void*>(p);
    return *p;
  }
  static void destroy(any& self) {
    Alloc a;
    Tp* p = static_cast<Tp*>(self.s_.ptr_);
    ATraits::destroy(a, p);
    ATraits::deallocate(a, p, sizeof(Tp));
    self.h_ = nullptr;
  }

  static void copy(const any& self, any& dst) {
    LargeHandle::create(
        dst, *static_cast<const Tp*>(static_cast<const void*>(&self.s_.buf_)));
  }
  static void move(any& self, any& dst) {
    dst.s_.ptr_ = std::exchange(self.s_.ptr_, nullptr);
    dst.h_ = std::exchange(self.h_, nullptr);
  }
  static void* get(any& self, const std::type_info* info,
                   const void* fallback_id) {
    if (any_impl::compare_typeid<Tp>(info, fallback_id)) {
      return static_cast<void*>(&self.s_.buf_);
    }
    return nullptr;
  }
  static void* typeinfo() {
    return const_cast<void*>(static_cast<const void*>(&typeid(Tp)));
  }
};

}  // namespace any_impl

template <class ValueType, class Tp, class>
any::any(ValueType&& v) : h_{nullptr} {
  any_impl::Handler<Tp>::create(*this, std::forward<ValueType>(v));
}

template <class ValueType, class... Args, class Tp, class>
any::any(std::in_place_type_t<ValueType>, Args&&... args) {
  any_impl::Handler<Tp>::create(*this, std::forward<Args>(args)...);
}

template <class ValueType, class Up, class... Args, class Tp, class>
any::any(std::in_place_type_t<ValueType>, std::initializer_list<Up> u,
         Args&&... args) {
  any_impl::Handler<Tp>::create(*this, u, std::forward<Args>(args)...);
}

template <class ValueType, class, class>
inline any& any::operator=(ValueType&& v) {
  any(std::forward<ValueType>(v)).swap(*this);
  return *this;
}

template <class ValueType, class... Args, class Tp, class>
inline Tp& any::emplace(Args&&... args) {
  reset();
  return any_impl::Handler<Tp>::create(*this, std::forward<Args>(args)...);
}

template <class ValueType, class Up, class... Args, class Tp, class>
inline Tp& any::emplace(std::initializer_list<Up> up, Args&&... args) {
  reset();
  return any_impl::Handler<Tp>::create(*this, up, std::forward<Args>(args)...);
}

inline void any::swap(any& other) noexcept {
  if (this == std::addressof(other)) {
    return;
  }
  if (h_ && other.h_) {
    any tmp;
    other.call(Action::Move, &tmp);
    this->call(Action::Move, &other);
    tmp.call(Action::Move, this);
  } else if (h_) {
    this->call(Action::Move, &other);
  } else if (other.h_) {
    other.call(Action::Move, this);
  }
}

inline void swap(any& lhs, any& rhs) noexcept { lhs.swap(rhs); }

template <class T, class... Args>
any make_any(Args&&... args) {
  return any(std::in_place_type<T>, std::forward<Args>(args)...);
}
template <class T, class U, class... Args>
any make_any(std::initializer_list<U> il, Args&&... args) {
  return any(std::in_place_type<T>, il, std::forward<Args>(args)...);
}

template <class ValueType>
ValueType any_cast(const any& operand) {
  using RawType = std::remove_cvref_t<ValueType>;
  static_assert(std::is_constructible_v<ValueType, const RawType&>,
                "ValueType is required to be a const lvalue reference "
                "or a CopyConstructible type");
  auto tmp = any_cast<const RawType>(std::addressof(operand));
  if (tmp == nullptr) {
    throw_bad_any_cast();
  }
  return static_cast<ValueType>(*tmp);
}
template <class ValueType>
ValueType any_cast(any& operand) {
  using RawType = std::remove_cvref_t<ValueType>;
  static_assert(std::is_constructible_v<ValueType, RawType&>,
                "ValueType is required to be a const lvalue reference "
                "or a CopyConstructible type");
  auto tmp = any_cast<RawType>(std::addressof(operand));
  if (tmp == nullptr) {
    throw_bad_any_cast();
  }
  return static_cast<ValueType>(*tmp);
}
template <class ValueType>
ValueType any_cast(any&& operand) {
  using RawType = std::remove_cvref_t<ValueType>;
  static_assert(std::is_constructible_v<ValueType, RawType>,
                "ValueType is required to be a const rvalue reference "
                "or a CopyConstructible type");
  auto tmp = any_cast<RawType>(std::addressof(operand));
  if (tmp == nullptr) {
    throw_bad_any_cast();
  }
  return static_cast<ValueType>(std::move(*tmp));
}
template <class ValueType>
const ValueType* any_cast(const any* operand) noexcept {
  static_assert(!std::is_void_v<ValueType>, "ValueType may not be void.");
  static_assert(!std::is_reference_v<ValueType>,
                "ValueType may not be reference.");
  return any_cast<ValueType>(const_cast<any*>(operand));
}

template <class RetType>
inline RetType pointer_or_func_cast(void* p,
                                    /*IsFunction*/ std::false_type) noexcept {
  return static_cast<RetType>(p);
}

template <class RetType>
inline RetType pointer_or_func_cast(void* p, std::true_type) noexcept {
  return nullptr;
}

template <class ValueType>
ValueType* any_cast(any* operand) noexcept {
  using any_impl::Action;
  static_assert(!std::is_void_v<ValueType>, "ValueType may not be void.");
  static_assert(!std::is_reference_v<ValueType>,
                "ValueType may not be reference.");
  using RetType = std::add_pointer_t<ValueType>;
  if (operand && operand->h_) {
    void* p = operand->call(Action::Get, nullptr, &typeid(ValueType),
                            any_impl::get_fallback_typeid<ValueType>());
    return pointer_or_func_cast<RetType>(p, std::is_function<ValueType>{});
  } else {
    return nullptr;
  }
}

}  // namespace mystd

#endif  // CH6_TYPE_ERASURE_ANY_ANY_HPP_