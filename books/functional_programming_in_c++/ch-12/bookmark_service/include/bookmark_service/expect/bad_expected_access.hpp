#ifndef BOOKMARK_SERVICE_EXPECT_BAD_EXPECTED_ACCESS_HPP_
#define BOOKMARK_SERVICE_EXPECT_BAD_EXPECTED_ACCESS_HPP_

#include <exception>

namespace bms {

template <typename Tp>
class bad_expected_access;

template <>
class bad_expected_access<void> : public std::exception {
 public:
  bad_expected_access() noexcept = default;
  bad_expected_access(const bad_expected_access&) = default;
  bad_expected_access(bad_expected_access&&) noexcept = default;
  bad_expected_access& operator=(const bad_expected_access&) = default;
  bad_expected_access& operator=(bad_expected_access&&) noexcept = default;
  ~bad_expected_access() override = default;
  [[nodiscard]] const char* what() const noexcept override {
    return "bms: bad expected access";
  }
};

template <typename Err>
class bad_expected_access : public bad_expected_access<void> {
 public:
  explicit bad_expected_access(Err e) : error_(std::move(e)) {}
  Err& error() & noexcept { return error_; }
  const Err& error() const& noexcept { return error_; }
  Err& error() && noexcept { return std::move(error_); }
  const Err& error() const&& noexcept { return std::move(error_); }

 private:
  Err error_;
};

}  // namespace bms

#endif  //  BOOKMARK_SERVICE_EXPECT_BAD_EXPECTED_ACCESS_HPP_
