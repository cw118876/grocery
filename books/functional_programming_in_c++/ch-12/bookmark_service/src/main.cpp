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
  auto receiver_cout = bms::receiver([](const auto& msg) {
    std::cout << msg << "\n";
  });
  auto pipeline = srv | receiver_cout;
  srv.start();
  (void)pipeline;
  io.run();

  return 0;
}
