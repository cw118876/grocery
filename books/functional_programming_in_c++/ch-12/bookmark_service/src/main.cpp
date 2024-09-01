#include <iostream>
#include <string>
#include "asio.hpp"
#include "bookmark_service/service.hpp"

int main(int argc, char* argv[]) {
  (void)argc;
  (void)argv;
  asio::io_context io;
  bms::service svr{io};
  svr.set_out_handler([](const auto& message){
    std::cout << message << "\n";
  });
  svr.start();
  io.run();

  return 0;
}
