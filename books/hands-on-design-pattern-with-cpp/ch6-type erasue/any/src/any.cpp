#include "any/any.hpp"

namespace mystd {

const char* bad_any_cast::what() const noexcept {
  return "bad any cast";
}

}  // namespace mystd
