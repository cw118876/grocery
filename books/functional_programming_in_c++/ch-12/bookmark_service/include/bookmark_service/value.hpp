#ifndef BOOKMARK_SERVICE_VALUE_HPP_
#define BOOKMARK_SERVICE_VALUE_HPP_
#include <functional>
#include <initializer_list>
#include <vector>

#include "bookmark_service/actor.hpp"

#include <iostream>

namespace bms {

namespace detail {}  // namespace detail

template <typename T>
class value {
 public:
  using out_type = T;
  value(std::initializer_list<T> init_list) : vec_{init_list} {}

  template <typename Handler>
  void set_out_handler(Handler h) {
    handler_ = h;
    std::for_each(vec_.cbegin(), vec_.cend(),
                  [&](T value) { h(std::move(value)); });
  }

 private:
  std::vector<out_type> vec_;
  std::function<void(out_type&&)> handler_{&detail::noop_out_handler<out_type>};
};

}  // namespace bms

#endif  // BOOKMARK_SERVICE_VALUE_HPP_
