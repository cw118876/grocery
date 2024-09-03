#include <iostream>
#include <string>
#include "asio.hpp"
#include "bookmark_service/filter.hpp"
#include "bookmark_service/receiver.hpp"
#include "bookmark_service/service.hpp"
#include "bookmark_service/transform.hpp"
#include "bookmark_service/trim.hpp"
#include "bookmark_service/value.hpp"

int main(int argc, char* argv[]) {
  (void)argc;
  (void)argv;
  // asio::io_context io;
  // auto srv = bms::service(io);
  // auto pipeline =
  //     srv |
  //     bms::transform(bms::trim) |
  //     bms::receiver([](const auto& msg) { std::cout << msg << "\n"; });
  // ;
  // srv.start();
  // (void)pipeline;
  // io.run();
  auto pipeline =
      bms::value{12, 31, 45, 123, 22, 1241, 112, 1, 4, 5} |
      bms::filter([](const auto& msg) { return !(msg & 1); }) |
      bms::receiver([](const auto& msg) { std::cout << msg << "\n"; });

  return 0;
}
