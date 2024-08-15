#ifndef BOOKMARK_SERVICE_EXPECTED_HPP_
#define BOOKMARK_SERVICE_EXPECTED_HPP_

#include <stdexcept>
#include <utility>
#include <functional>

struct in_place_t {};
struct unexpected_t {};

template <typename Tp, typename Err>
class expected {
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
      if (other.has_value()) {
        auto val = other.value_;
        clean_up();
        has_val_ = true;
        new (&value_) Tp{std::move(val)};
      } else {
        auto err = other.error_;
        clean_up();
        has_val_ = false;
        new (&error_) Err(std::move(err));
      }
    }

    return *this;
  }
  expected& operator=(expected&& other) noexcept {
    if (this != std::addressof(other)) {
      swap(other);
    }
    return *this;
  }

  template <typename... Args>
  static expected emplace_value(Args&&... args) {
    return {in_place_t{}, std::forward<Args>(args)...};
  }

  template <typename... Args>
  static expected emplace_error(Args&&... args) {
    return {unexpected_t{}, std::forward<Args>(args)...};
  }

  Tp& value() & {
    if (!has_value()) {
      throw std::invalid_argument(
          "cannot obtain value due to that expected is unexpected");
    }
    return value_;
  }

  const Tp& value() const& {
    if (!has_value()) {
      throw std::invalid_argument(
          "cannot obtain value due to that expected is unexpected");
    }
    return value_;
  }

  Tp&& value() && {
    if (!has_value()) {
      throw std::invalid_argument(
          "cannot obtain value due to that expected is unexpected");
    }
    return std::move(value_);
  }

  const Tp&& value() const&& {
    if (!has_value()) {
      throw std::invalid_argument(
          "cannot obtain value due to that expected is unexpected");
    }
    return std::move(value_);
  }

  Tp* operator->() { return &value(); }
  const Tp* operator->() const { return &value(); }

  Err& error() & {
    if (has_value()) {
      throw std::invalid_argument(
          "cannot obtain error because of valid value state");
    }
    return error_;
  }
  const Err& error() const& {
    if (has_value()) {
      throw std::invalid_argument(
          "cannot obtain error because of valid value state");
    }
    return error_;
  }
  Err&& error() && {
    if (has_value()) {
      throw std::invalid_argument(
          "cannot obtain error because of valid value state");
    }
    return std::move(error_);
  }
  const Err&& error() const&& {
    if (has_value()) {
      throw std::invalid_argument(
          "cannot obtain error because of valid value state");
    }
    return std::move(error_);
  }
  template <typename Fun>
  void visit(Fun fun) {
    if (has_value()) {
        std::invoke(fun, value_);
    } else {
        std::invoke(fun, error_);
    }
  }
};

#endif  // BOOKMARK_SERVICE_EXPECTED_HPP_
