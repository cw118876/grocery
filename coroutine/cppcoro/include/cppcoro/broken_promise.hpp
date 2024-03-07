#ifndef CPPCORO_BROKEN_PROMISE_HPP_
#define CPPCORO_BROKEN_PROMISE_HPP_

#include <stdexcept>

namespace coro {

// @brief Exception thrown when you attempt to retrieve the result of
// a task that has been detached from its promise/coroutine
class broken_promise : public std::logic_error {
 public:
  broken_promise() : std::logic_error{"broken promise"} {}
};
}  // namespace coro

#endif  // CPPCORO_BROKEN_PROMISE_HPP_
