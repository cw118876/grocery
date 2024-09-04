#ifndef BOOKMARK_SERVICE_MTRY_HPP_
#define BOOKMARK_SERVICE_MTRY_HPP_

#include <exception>
#include <type_traits>
#include "bookmark_service/expect/expected.hpp"

namespace bms {

template <typename F,
          typename Ret = std::invoke_result_t<F>,
          typename Exp = expected<Ret, std::exception_ptr>>
Exp mtry(F f) {
  try {
    return Exp::emplace_value(f());
  } catch (...) {
    return Exp::emplace_error(std::current_exception());
  }
}

}  // namespace bms

#endif  // BOOKMARK_SERVICE_MTRY_HPP_
