#ifndef ANY_TEST_ANY_HELPER_H_
#define ANY_TEST_ANY_HELPER_H_
#include <future>
#include <initializer_list>
#include <iostream>
#include <type_traits>
#include <typeinfo>
#include <utility>

#include "any/any.hpp"

template <class Tp>
struct IsSmallObject
    : public std::integral_constant<
          bool,
          sizeof(Tp) <= sizeof(mystd::any) - sizeof(void*) &&
              std::alignment_of_v<void*> % std::alignment_of_v<Tp> == 0 &&
              std::is_nothrow_move_constructible_v<Tp>> {};

template <class Tp>
bool containsType(const mystd::any& a) {
  return a.type() == typeid(Tp);
}

template <class Tp>
bool isSmallType() {
  return IsSmallObject<Tp>::value;
}

// declval<T&> is create an lvalue-reference, for overload function , ref
// member function
template <class Type>
constexpr auto has_value_member(int)
    -> decltype(std::declval<Type&>().value, true) {
  return true;
}

template <class>
constexpr auto has_value_member(long) {
  return false;
}

template <int Dummy = 0>
struct small_type {
  inline static int count = 0;
  inline static int copied = 0;
  inline static int moved = 0;
  inline static int const_copied = 0;
  inline static int non_const_copied = 0;
  static void reset() {
    small_type::const_copied = 0;
    small_type::count = 0;
    small_type::copied = 0;
    small_type::moved = 0;
    small_type::non_const_copied = 0;
  }
  explicit small_type(int val = 0) : value_(val) { ++count; }
  explicit small_type(int, int val, int) : value_(val) { ++count; }
  small_type(std::initializer_list<int> il) : value_(*il.begin()) { ++count; }

  small_type(const small_type& other) noexcept {
    value_ = other.value_;
    ++count;
    ++copied;
    ++const_copied;
  }

  small_type(small_type& other) noexcept {
    value_ = other.value_;
    other.value_ = 0;
    ++count;
    ++copied;
    ++non_const_copied;
  }

  small_type(small_type&& other) noexcept {
    value_ = other.value_;
    other.value_ = 0;
    ++count;
    ++moved;
  }
  ~small_type() {
    value_ = -1;
    --count;
  }
  int value_ = 0;

 private:
  small_type& operator=(const small_type&) = delete;
  small_type& operator=(small_type&&) = delete;
};

using small = small_type<>;
using small1 = small_type<1>;
using small2 = small_type<2>;

template <int Dummy = 0>
struct large_type {
  inline static int count = 0;
  inline static int copied = 0;
  inline static int movied = 0;
  inline static int const_copied = 0;
  inline static int non_const_copied = 0;

  static void reset() {
    count = 0;
    movied = 0;
    copied = 0;
    const_copied = 0;
    non_const_copied = 0;
  }
  large_type(int val = 0) : value_(val) {
    ++count;
    data_[0] = 0;
  }
  large_type(int, int val, int) : value_(val) {
    ++count;
    data_[0] = 0;
  }
  large_type(std::initializer_list<int> il) : value_(*il.begin()) {
    ++count;
    data_[0] = 0;
  }
  large_type(const large_type& other) noexcept {
    value_ = other.value_;
    ++count;
    ++copied;
    ++const_copied;
  }
  large_type(large_type& other) noexcept {
    value_ = other.value_;
    ++count;
    ++copied;
    ++non_const_copied;
  }
  large_type(large_type&& other) noexcept {
    value_ = other.value_;
    other.value_ = 0;
    ++count;
    ++movied;
  }
  ~large_type() {
    value_ = 0;
    --count;
  }
  int value_ = 0;

 private:
  inline static constexpr size_t kNumElement = 16;
  int data_[kNumElement];
  large_type& operator=(const large_type&) = delete;
  large_type& operator=(large_type&&) = delete;
};

using large = large_type<>;
using large1 = large_type<1>;
using large2 = large_type<2>;

struct my_any_exception {};
void throwMyAnyException() {
  throw my_any_exception{};
}

struct small_throws_on_copy {
  inline static int count = 0;
  inline static int copied = 0;
  inline static int movied = 0;
  static void reset() {
    count = 0;
    copied = 0;
    movied = 0;
  }
  int value_ = 0;
  explicit small_throws_on_copy(int val = 0) : value_(val) { ++count; }
  small_throws_on_copy(int, int val, int) : value_(val) { ++count; }
  small_throws_on_copy(const small_throws_on_copy&) { throwMyAnyException(); }
  small_throws_on_copy(small_throws_on_copy&& other) throw() {
    value_ = other.value_;
    ++count;
    ++movied;
  }
  ~small_throws_on_copy() {
    value_ = 0;
    --count;
  }

 private:
  small_throws_on_copy& operator=(const small_throws_on_copy&) = delete;
  small_throws_on_copy& operator=(small_throws_on_copy&&) = delete;
};

struct large_throws_on_copy {
  inline static int count = 0;
  inline static int copied = 0;
  inline static int movied = 0;
  static void reset() {
    count = 0;
    copied = 0;
    movied = 0;
  }
  int value_ = 0;
  explicit large_throws_on_copy(int val = 0) : value_(val) { ++count; }
  large_throws_on_copy(int, int val, int) : value_(val) { ++count; }
  large_throws_on_copy(const large_throws_on_copy&) { throwMyAnyException(); }
  large_throws_on_copy(large_throws_on_copy&& other) throw() {
    value_ = other.value_;
    ++count;
    ++movied;
  }
  ~large_throws_on_copy() {
    value_ = 0;
    --count;
  }

 private:
  inline static constexpr size_t kNumElement = 16;
  int data_[kNumElement];
  large_throws_on_copy& operator=(const large_throws_on_copy&) = delete;
  large_throws_on_copy& operator=(large_throws_on_copy&&) = delete;
};

struct throws_on_move {
  inline static int count = 0;
  inline static int copied = 0;
  inline static int movied = 0;
  static void reset() {
    count = 0;
    copied = 0;
    movied = 0;
  }
  int value_ = 0;
  explicit throws_on_move(int val = 0) : value_(val) { ++count; }
  throws_on_move(int, int val, int) : value_(val) { ++count; }
  throws_on_move(const throws_on_move& other) {
    value_ = other.value_;
    ++count;
    ++copied;
  }
  throws_on_move(throws_on_move&& other) { throwMyAnyException(); }
  ~throws_on_move() {
    value_ = 0;
    --count;
  }

 private:
  throws_on_move& operator=(const throws_on_move&) = delete;
  throws_on_move& operator=(throws_on_move&&) = delete;
};

#endif  // ANY_TEST_ANY_HELPER_H_