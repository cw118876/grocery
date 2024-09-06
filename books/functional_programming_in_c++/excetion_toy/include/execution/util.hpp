#ifndef EXECUTION_UTIL_HPP_
#define EXECUTION_UTIL_HPP_

#include <functional>
#include <exception>

namespace ex {

using Err = std::exception_ptr;
template <typename Sender, typename Receiver>
using operator_t =
    decltype(connect(std::declval<Sender>(), std::declval<Receiver>()));

template <typename Sender>
using sender_result_t = typename Sender::result_type;

struct immovable {
  immovable() = default;
  immovable(immovable&&) = delete;
};

} // namespace ex



#endif   // EXECUTION_UTIL_HPP_
