#include <iostream>
#include <string>
#include "asio.hpp"
#include "bookmark_service/receiver.hpp"
#include "bookmark_service/service.hpp"
#include "bookmark_service/transform.hpp"
#include "bookmark_service/trim.hpp"

int main(int argc, char* argv[]) {
  (void)argc;
  (void)argv;
  asio::io_context io;
  auto srv = bms::service(io);
  auto pipeline =
      srv |
      bms::transform(bms::trim) |
      bms::receiver([](const auto& msg) { std::cout << msg << "\n"; });
  ;
  srv.start();
  (void)pipeline;
  io.run();

  return 0;
}
