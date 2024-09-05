#include <iostream>

#include "execution/execution.hpp"

int main(int argc, const char* argv[]) {

  (void) argc;
  (void) argv;
  auto s2 = ex::then(ex::just(123), [](auto&& i) { return i + 42; });
  auto s4 = ex::sync_wait(s2);
  std::cout << std::get<1>(s4) << "\n";

  return 0;
}