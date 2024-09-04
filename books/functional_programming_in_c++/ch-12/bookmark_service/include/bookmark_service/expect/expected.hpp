#ifndef BOOKMARK_SERVICE_EXPECT_EXPECTED_HPP_
#define BOOKMARK_SERVICE_EXPECT_EXPECTED_HPP_

#include <functional>
#include <stdexcept>
#include <type_traits>
#include <utility>

#include "bookmark_service/expect/bad_expected_access.hpp"

#if __cplusplus < 202002L
namespace std {

template <class T>
struct remove_cvref {
  using type = std::remove_cv_t<std::remove_reference_t<T>>;
};

template <class T>
using remove_cvref_t = typename remove_cvref<T>::type;

}  // namespace std
#endif

namespace bms {

struct in_place_t {};
struct unexpected_t {};

// forward declared expected and unexpected
template <typename Err>
class unexpected;
template <typename Tp, typename Err>
class expected;

template <typename Err, typename... Args>
void throw_bad_expected_access(Args&&... args) {
  throw bad_expected_access<Err>(std::forward<Args>(args)...);
}

template <typename T>
struct is_generated_from_expected : std::false_type {};

template <typename T, typename Err>
struct is_generated_from_expected<expected<T, Err>> : std::true_type {};

template <typename Tp, typename Err>
class expected {
  static_assert(!std::is_reference_v<Tp> && !std::is_function_v<Tp> &&
                    !std::is_same_v<std::remove_cvref_t<Tp>, in_place_t>,
                "expected instantiates for reference, function, in_place_t "
                "will be ill-formed.");

 private:
  union {
    Tp value_;
    Err error_;
  };
  bool has_val_ = true;

  void clean_up() {
    if (has_val_) {
      value_.~Tp();
    } else {
      error_.~Err();
    }
  }

 public:
  using value_type = Tp;
  using error_type = Err;
  using unexpected_type = unexpected<Err>;

  template <typename U>
  using rebind = expected<U, Err>;

  expected() : value_(Tp{}) {}

  template <typename... Args>
  explicit expected(in_place_t, Args&&... args)
      : value_(Tp{std::forward<Args>(args)...}) {}

  template <typename... Args>
  explicit expected(unexpected_t, Args&&... args)
      : error_(Err{std::forward<Args>(args)...}), has_val_(false) {}

  ~expected() { clean_up(); }
  expected(const expected& other) : has_val_(other.has_val_) {
    if (has_val_) {
      new (&value_) Tp{other.value_};
    } else {
      new (&error_) Err{other.error_};
    }
  }
  expected(expected&& other) noexcept : has_val_(other.has_val_) {
    if (has_val_) {
      new (&value_) Tp{std::move(other.value_)};
    } else {
      new (&error_) Err{std::move(other.error_)};
    }
  }
  [[nodiscard]] bool has_value() const noexcept { return has_val_; }
  explicit operator bool() const noexcept { return has_val_; }

  void swap(expected& other) {
    using std::swap;
    if (has_value()) {
      if (other.has_value()) {
        // both are valid
        swap(value_, other.value_);
      } else {
        auto temp = std::move(other.error_);
        other.error_.~Err();
        new (&other.value_) Tp(std::move(value_));
        value_.~Tp();
        new (&error_) Err(std::move(temp));
        swap(has_val_, other.has_val_);
      }

    } else {
      if (other.has_value()) {
        other.swap(*this);
      } else {
        swap(error_, other.error_);
      }
    }
  }
  expected& operator=(const expected& other) {
    if (this != std::addressof(other)) {
      expected temp{other};
      swap(temp);
    }

    return *this;
  }
  expected& operator=(expected&& other) noexcept {
    if (this != std::addressof(other)) {
      expected temp{std::move(other)};
      swap(temp);
    }
    return *this;
  }

  template <typename... Args>
  static expected emplace_value(Args&&... args) {
    return expected{in_place_t{}, std::forward<Args>(args)...};
  }

  template <typename... Args>
  static expected emplace_error(Args&&... args) {
    return expected{unexpected_t{}, std::forward<Args>(args)...};
  }

  Tp& value() & {
    if (!has_value()) {
      throw_bad_expected_access<Err>(error_);
    }
    return value_;
  }

  const Tp& value() const& {
    if (!has_value()) {
      throw_bad_expected_access<Err>(error_);
    }
    return value_;
  }

  Tp&& value() && {
    if (!has_value()) {
      throw_bad_expected_access<Err>(error_);
    }
    return std::move(value_);
  }

  const Tp&& value() const&& {
    if (!has_value()) {
      throw_bad_expected_access<Err>(error_);
    }
    return std::move(value_);
  }
  template <typename U>
  constexpr Tp value_or(U&& default_value) const& {
    static_assert(std::is_copy_constructible_v<Tp>,
                  "expected value must be copy constructible");
    static_assert(std::is_convertible_v<U, Tp>,
                  "argument must be convertible to value type");
    if (!has_value()) {
      return default_value;
    }
    return value_;
  }
  template <typename U>
  constexpr Tp value_or(U&& default_value) && {
    static_assert(std::is_move_constructible_v<Tp>,
                  "expected value must be move constructible");
    static_assert(std::is_convertible_v<U, Tp>,
                  "argument must be convertible to value type");
    if (!has_value()) {
      return default_value;
    }
    return std::move(value_);
  }

  // Bellow operations do not check whether the optional represents an expected
  // or check; you can have by using have_value or value_or
  Tp* operator->() { return std::addressof(value_); }
  const Tp* operator->() const { return std::addressof(value_); }

  constexpr const Tp& operator*() const& noexcept { return value_; }

  constexpr Tp& operator*() & noexcept { return value_; }

  constexpr const Tp&& operator*() const&& noexcept {
    return std::move(value_);
  }

  constexpr Tp&& operator*() && noexcept { return std::move(value_); }

  Err& error() & {
    if (has_value()) {
      throw_bad_expected_access<void>();
    }
    return error_;
  }
  const Err& error() const& {
    if (has_value()) {
      throw_bad_expected_access<void>();
    }
    return error_;
  }
  Err&& error() && {
    if (has_value()) {
      throw_bad_expected_access<void>();
    }
    return std::move(error_);
  }
  const Err&& error() const&& {
    if (has_value()) {
      throw_bad_expected_access<void>();
    }
    return std::move(error_);
  }
  template <typename G = Err>
  constexpr Err error_or(G&& default_value) const& {
    static_assert(std::is_copy_constructible_v<Err>,
                  "expected error type must be move constructible");
    static_assert(std::is_convertible_v<G, Err>,
                  "argument must be convertible to error type");
    if (!has_value()) {
      return error_;
    }
    return default_value;
  }
  template <typename G = Err>
  constexpr Err error_or(G&& default_value) && {
    static_assert(std::is_copy_constructible_v<Err>,
                  "expected error type must be move constructible");
    static_assert(std::is_convertible_v<G, Err>,
                  "argument must be convertible to error type");
    if (!has_value()) {
      return std::move(error_);
    }
    return default_value;
  }

  template <class F,
            typename = std::enable_if_t<std::is_constructible_v<Err, Err&>>>
  constexpr auto and_then(F&& f) & {
    using U = std::remove_cvref_t<std::invoke_result_t<F, Tp&>>;
    static_assert(is_generated_from_expected<U>::value,
                  "result type of F with value type"
                  " should be void or expected template class generate");
    static_assert(std::is_same_v<typename U::error_type, Err>,
                  "error type of F should be same as error type of expected");
    if (!has_value()) {
      return U(unexpected_t{}, error_);
    }
    return U{in_place_t{}, std::invoke(std::forward<F>(f), value_)};
  }

  template <
      class F,
      typename = std::enable_if_t<std::is_constructible_v<Err, const Err&>>>
  constexpr auto and_then(F&& f) const& {
    using U = std::remove_cvref_t<std::invoke_result_t<F, const Tp&>>;
    static_assert(is_generated_from_expected<U>::value,
                  "result type of F with value type"
                  " should be void or expected template class generate");
    static_assert(std::is_same_v<typename U::error_type, Err>,
                  "error type of F should be same as error type of expected");
    if (!has_value()) {
      return U(unexpected_t{}, error_);
    }
    return U{in_place_t{}, std::invoke(std::forward<F>(f), value_)};
  }

  template <class F,
            typename = std::enable_if_t<std::is_constructible_v<Err, Err&&>>>
  constexpr auto and_then(F&& f) && {
    using U = std::remove_cvref_t<std::invoke_result_t<F, Tp&&>>;
    static_assert(is_generated_from_expected<U>::value,
                  "result type of F with value type"
                  " should be void or expected template class generate");
    static_assert(std::is_same_v<typename U::error_type, Err>,
                  "error type of F should be same as error type of expected");
    if (!has_value()) {
      return U(unexpected_t{}, std::move(error_));
    }
    return U{in_place_t{}, std::invoke(std::forward<F>(f), std::move(value_))};
  }

  template <
      class F,
      typename = std::enable_if_t<std::is_constructible_v<Err, const Err&&>>>
  constexpr auto and_then(F&& f) const&& {
    using U = std::remove_cvref_t<std::invoke_result_t<F, const Tp&&>>;
    static_assert(is_generated_from_expected<U>::value,
                  "result type of F with value type"
                  " should be void or expected template class generate");
    static_assert(std::is_same_v<typename U::error_type, Err>,
                  "error type of F should be same as error type of expected");
    if (!has_value()) {
      return U(unexpected_t{}, std::move(error_));
    }
    return U{in_place_t{}, std::invoke(std::forward<F>(f), std::move(value_))};
  }

  template <class F,
            typename = std::enable_if_t<std::is_constructible_v<Err, Err&>>>
  constexpr auto transform(F&& f) & {
    using U = std::remove_cvref_t<std::invoke_result_t<F, Tp&>>;
    if (!has_value()) {
      return expected<U, Err>(unexpected_t{}, error_);
    }
    if constexpr (std::is_void_v<U>) {
      std::invoke(std::forward<F>(f), value_);
      return expected<U, Err>();
    } else {
      return expected<U, Err>{in_place_t{},
                              std::invoke(std::forward<F>(f), value_)};
    }
  }

  template <
      class F,
      typename = std::enable_if_t<std::is_constructible_v<Err, const Err&>>>
  constexpr auto transform(F&& f) const& {
    using U = std::remove_cvref_t<std::invoke_result_t<F, const Tp&>>;
    if (!has_value()) {
      return expected<U, Err>(unexpected_t{}, error_);
    }
    if constexpr (std::is_void_v<U>) {
      std::invoke(std::forward<F>(f), value_);
      return expected<U, Err>();
    } else {
      return expected<U, Err>{in_place_t{},
                              std::invoke(std::forward<F>(f), value_)};
    }
  }

  template <class F,
            typename = std::enable_if_t<std::is_constructible_v<Err, Err&&>>>
  constexpr auto transform(F&& f) && {
    using U = std::remove_cvref_t<std::invoke_result_t<F, Tp&&>>;
    if (!has_value()) {
      return expected<U, Err>(unexpected_t{}, std::move(error_));
    }
    if constexpr (std::is_void_v<U>) {
      std::invoke(std::forward<F>(f), std::move(value_));
      return expected<U, Err>();
    } else {
      return expected<U, Err>{
          in_place_t{}, std::invoke(std::forward<F>(f), std::move(value_))};
    }
  }

  template <
      class F,
      typename = std::enable_if_t<std::is_constructible_v<Err, const Err&&>>>
  constexpr auto transform(F&& f) const&& {
    using U = std::remove_cvref_t<std::invoke_result_t<F, Tp&&>>;
    if (!has_value()) {
      return expected<U, Err>(unexpected_t{}, std::move(error_));
    }
    if constexpr (std::is_void_v<U>) {
      std::invoke(std::forward<F>(f), std::move(value_));
      return expected<U, Err>();
    } else {
      return expected<U, Err>{
          in_place_t{}, std::invoke(std::forward<F>(f), std::move(value_))};
    }
  }

  template <class F,
            typename = std::enable_if_t<std::is_constructible_v<Tp, Tp&>>>
  constexpr auto or_else(F&& f) & {
    using U = std::remove_cvref_t<std::invoke_result_t<F, Err&>>;
    static_assert(is_generated_from_expected<U>::value,
                  "error type of F with value type"
                  " should be void or expected template class generate");
    static_assert(std::is_same_v<typename U::value_type, Tp>,
                  "value type of F should be same as value type of expected");
    if (!has_value()) {
      return U{unexpected_t{}, std::invoke(std::forward<F>(f), error_)};
    }
    return U{in_place_t{}, value_};
  }

  template <class F,
            typename = std::enable_if_t<std::is_constructible_v<Tp, const Tp&>>>
  constexpr auto or_else(F&& f) const& {
    using U = std::remove_cvref_t<std::invoke_result_t<F, const Err&>>;
    static_assert(is_generated_from_expected<U>::value,
                  "error type of F with value type"
                  " should be void or expected template class generate");
    static_assert(std::is_same_v<typename U::value_type, Tp>,
                  "value type of F should be same as value type of expected");
    if (!has_value()) {
      return U{unexpected_t{}, std::invoke(std::forward<F>(f), error_)};
    }
    return U{in_place_t{}, value_};
  }

  template <class F,
            typename = std::enable_if_t<std::is_constructible_v<Tp, Tp&&>>>
  constexpr auto or_else(F&& f) && {
    using U = std::remove_cvref_t<std::invoke_result_t<F, Err&&>>;
    static_assert(is_generated_from_expected<U>::value,
                  "error type of F with value type"
                  " should be void or expected template class generate");
    static_assert(std::is_same_v<typename U::value_type, Tp>,
                  "value type of F should be same as value type of expected");
    if (!has_value()) {
      return U{unexpected_t{},
               std::invoke(std::forward<F>(f), std::move(error_))};
    }
    return U{in_place_t{}, std::move(value_)};
  }

  template <
      class F,
      typename = std::enable_if_t<std::is_constructible_v<Tp, const Tp&&>>>
  constexpr auto or_else(F&& f) const&& {
    using U = std::remove_cvref_t<std::invoke_result_t<F, const Err&&>>;
    static_assert(is_generated_from_expected<U>::value,
                  "error type of F with value type"
                  " should be void or expected template class generate");
    static_assert(std::is_same_v<typename U::value_type, Tp>,
                  "value type of F should be same as value type of expected");
    if (!has_value()) {
      return U{unexpected_t{},
               std::invoke(std::forward<F>(f), std::move(error_))};
    }
    return U{in_place_t{}, std::move(value_)};
  }
};

template <typename Err>
class expected<void, Err> {
 public:
  using value_type = void;
  using error_type = Err;
  using unexpected_type = unexpected<Err>;

  template <typename U>
  using rebind = expected<U, Err>;

  expected() = default;

  template <typename... Args>
  explicit expected(in_place_t, Args&&... args) {}

  template <typename... Args>
  explicit expected(unexpected_t, Args&&... args)
      : error_(Err{std::forward<Args>(args)...}), has_val_(false) {}

  ~expected() { clean_up(); }
  expected(const expected& other) : has_val_(other.has_val_) {
    if (!has_val_) {
      new (&error_) Err{other.error_};
    }
  }
  expected(expected&& other) noexcept : has_val_(other.has_val_) {
    if (!has_val_) {
      new (&error_) Err{std::move(other.error_)};
    }
  }
  [[nodiscard]] bool has_value() const noexcept { return has_val_; }
  explicit operator bool() const noexcept { return has_val_; }

  void swap(expected& other) {
    using std::swap;
    if (has_value()) {
      if (other.has_value()) {
        // both are unexpected
      } else {
        auto temp = std::move(other.error_);
        other.error_.~Err();
        new (&error_) Err(std::move(temp));
        swap(has_val_, other.has_val_);
      }
    } else {
      if (other.has_value()) {
        other.swap(*this);
      } else {
        swap(error_, other.error_);
      }
    }
  }
  expected& operator=(const expected& other) {
    if (this != std::addressof(other)) {
      expected temp{other};
      swap(temp);
    }

    return *this;
  }
  expected& operator=(expected&& other) noexcept {
    if (this != std::addressof(other)) {
      expected temp{std::move(other)};
      swap(temp);
    }
    return *this;
  }

  template <typename... Args>
  static expected emplace_value(Args&&... args) {
    (void)sizeof...(args);
    return {in_place_t{}};
  }

  template <typename... Args>
  static expected emplace_error(Args&&... args) {
    return {unexpected_t{}, std::forward<Args>(args)...};
  }

  void value() const& {
    if (!has_value()) {
      throw_bad_expected_access<Err>(error_);
    }
    return;
  }

  void value() && {
    if (!has_value()) {
      throw_bad_expected_access<Err>(error_);
    }
    return;
  }

  constexpr void operator->() const { return; }

  Err& error() & {
    if (has_value()) {
      throw_bad_expected_access<void>();
    }
    return error_;
  }
  const Err& error() const& {
    if (has_value()) {
      throw_bad_expected_access<void>();
    }
    return error_;
  }
  Err&& error() && {
    if (has_value()) {
      throw_bad_expected_access<void>();
    }
    return std::move(error_);
  }
  const Err&& error() const&& {
    if (has_value()) {
      throw_bad_expected_access<void>();
    }
    return std::move(error_);
  }
  template <typename G = Err>
  constexpr Err error_or(G&& default_value) const& {
    static_assert(std::is_copy_constructible_v<Err>,
                  "expected error type must be move constructible");
    static_assert(std::is_convertible_v<G, Err>,
                  "argument must be convertible to error type");
    if (!has_value()) {
      return error_;
    }
    return default_value;
  }
  template <typename G = Err>
  constexpr Err error_or(G&& default_value) && {
    static_assert(std::is_copy_constructible_v<Err>,
                  "expected error type must be move constructible");
    static_assert(std::is_convertible_v<G, Err>,
                  "argument must be convertible to error type");
    if (!has_value()) {
      return std::move(error_);
    }
    return default_value;
  }

  template <class F,
            typename = std::enable_if_t<std::is_constructible_v<Err, Err&>>>
  constexpr auto and_then(F&& f) & {
    using U = std::remove_cvref_t<std::invoke_result_t<F>>;
    static_assert(is_generated_from_expected<U>::value,
                  "result type of F with value type"
                  " should be void or expected template class generate");
    static_assert(std::is_same_v<typename U::error_type, Err>,
                  "error type of F should be same as error type of expected");
    if (!has_value()) {
      return U(unexpected_t{}, error_);
    }
    return U{in_place_t{}, std::invoke(std::forward<F>(f))};
  }

  template <
      class F,
      typename = std::enable_if_t<std::is_constructible_v<Err, const Err&>>>
  constexpr auto and_then(F&& f) const& {
    using U = std::remove_cvref_t<std::invoke_result_t<F>>;
    static_assert(is_generated_from_expected<U>::value,
                  "result type of F with value type"
                  " should be void or expected template class generate");
    static_assert(std::is_same_v<typename U::error_type, Err>,
                  "error type of F should be same as error type of expected");
    if (!has_value()) {
      return U(unexpected_t{}, error_);
    }
    return U{in_place_t{}, std::invoke(std::forward<F>(f))};
  }

  template <class F,
            typename = std::enable_if_t<std::is_constructible_v<Err, Err&&>>>
  constexpr auto and_then(F&& f) && {
    using U = std::remove_cvref_t<std::invoke_result_t<F>>;
    static_assert(is_generated_from_expected<U>::value,
                  "result type of F with value type"
                  " should be void or expected template class generate");
    static_assert(std::is_same_v<typename U::error_type, Err>,
                  "error type of F should be same as error type of expected");
    if (!has_value()) {
      return U(unexpected_t{}, std::move(error_));
    }
    return U{in_place_t{}, std::invoke(std::forward<F>(f))};
  }

  template <
      class F,
      typename = std::enable_if_t<std::is_constructible_v<Err, const Err&&>>>
  constexpr auto and_then(F&& f) const&& {
    using U = std::remove_cvref_t<std::invoke_result_t<F>>;
    static_assert(is_generated_from_expected<U>::value,
                  "result type of F with value type"
                  " should be void or expected template class generate");
    static_assert(std::is_same_v<typename U::error_type, Err>,
                  "error type of F should be same as error type of expected");
    if (!has_value()) {
      return U(unexpected_t{}, std::move(error_));
    }
    return U{in_place_t{}, std::invoke(std::forward<F>(f))};
  }

  template <class F,
            typename = std::enable_if_t<std::is_constructible_v<Err, Err&>>>
  constexpr auto transform(F&& f) & {
    using U = std::remove_cvref_t<std::invoke_result_t<F>>;
    if (!has_value()) {
      return expected<U, Err>(unexpected_t{}, error_);
    }
    if constexpr (std::is_void_v<U>) {
      std::invoke(std::forward<F>(f));
      return expected<U, Err>();
    } else {
      return expected<U, Err>{in_place_t{}, std::invoke(std::forward<F>(f))};
    }
  }

  template <
      class F,
      typename = std::enable_if_t<std::is_constructible_v<Err, const Err&>>>
  constexpr auto transform(F&& f) const& {
    using U = std::remove_cvref_t<std::invoke_result_t<F>>;
    if (!has_value()) {
      return expected<U, Err>(unexpected_t{}, error_);
    }
    if constexpr (std::is_void_v<U>) {
      std::invoke(std::forward<F>(f));
      return expected<U, Err>();
    } else {
      return expected<U, Err>{in_place_t{}, std::invoke(std::forward<F>(f))};
    }
  }

  template <class F,
            typename = std::enable_if_t<std::is_constructible_v<Err, Err&&>>>
  constexpr auto transform(F&& f) && {
    using U = std::remove_cvref_t<std::invoke_result_t<F>>;
    if (!has_value()) {
      return expected<U, Err>(unexpected_t{}, std::move(error_));
    }
    if constexpr (std::is_void_v<U>) {
      std::invoke(std::forward<F>(f));
      return expected<U, Err>();
    } else {
      return expected<U, Err>{in_place_t{}, std::invoke(std::forward<F>(f))};
    }
  }

  template <
      class F,
      typename = std::enable_if_t<std::is_constructible_v<Err, const Err&&>>>
  constexpr auto transform(F&& f) const&& {
    using U = std::remove_cvref_t<std::invoke_result_t<F>>;
    if (!has_value()) {
      return expected<U, Err>(unexpected_t{}, std::move(error_));
    }
    if constexpr (std::is_void_v<U>) {
      std::invoke(std::forward<F>(f));
      return expected<U, Err>();
    } else {
      return expected<U, Err>{in_place_t{}, std::invoke(std::forward<F>(f))};
    }
  }

  template <class F>
  constexpr auto or_else(F&& f) & {
    using U = std::remove_cvref_t<std::invoke_result_t<F, Err&>>;
    static_assert(is_generated_from_expected<U>::value,
                  "error type of F with value type"
                  " should be void or expected template class generate");
    if (!has_value()) {
      return U{unexpected_t{}, std::invoke(std::forward<F>(f), error_)};
    }
    return U{in_place_t{}};
  }

  template <class F>
  constexpr auto or_else(F&& f) const& {
    using U = std::remove_cvref_t<std::invoke_result_t<F, const Err&>>;
    static_assert(is_generated_from_expected<U>::value,
                  "error type of F with value type"
                  " should be void or expected template class generate");
    if (!has_value()) {
      return U{unexpected_t{}, std::invoke(std::forward<F>(f), error_)};
    }
    return U{in_place_t{}};
  }

  template <class F>
  constexpr auto or_else(F&& f) && {
    using U = std::remove_cvref_t<std::invoke_result_t<F, Err&&>>;
    static_assert(is_generated_from_expected<U>::value,
                  "error type of F with value type"
                  " should be void or expected template class generate");
    if (!has_value()) {
      return U{unexpected_t{},
               std::invoke(std::forward<F>(f), std::move(error_))};
    }
    return U{in_place_t{}};
  }

  template <class F>
  constexpr auto or_else(F&& f) const&& {
    using U = std::remove_cvref_t<std::invoke_result_t<F, const Err&&>>;
    static_assert(is_generated_from_expected<U>::value,
                  "error type of F with value type"
                  " should be void or expected template class generate");
    if (!has_value()) {
      return U{unexpected_t{},
               std::invoke(std::forward<F>(f), std::move(error_))};
    }
    return U{in_place_t{}};
  }

 private:
  union {
    Err error_;
  };
  bool has_val_ = true;
  void clean_up() {
    if (!has_val_) {
      error_.~Err();
    }
  }
};

}  // namespace bms

#endif  // BOOKMARK_SERVICE_EXPECT_EXPECTED_HPP_
