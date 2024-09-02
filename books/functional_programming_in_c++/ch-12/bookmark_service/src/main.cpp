#include <iostream>
#include <string>
#include "asio.hpp"
#include "bookmark_service/receiver.hpp"
#include "bookmark_service/service.hpp"

int main(int argc, char* argv[]) {
  (void)argc;
  (void)argv;
  asio::io_context io;
  auto srv = bms::service(io);
  auto pipeline = bms::make_receiver(srv, [](const auto& msg) {
    std::cout << msg << "\n";
  });
  srv.start();
  (void)pipeline;
  io.run();

  return 0;
}
