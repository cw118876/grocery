#include <iostream>
#include <string>
#include "asio.hpp"
#include "bookmark_service/expect/expected.hpp"
#include "bookmark_service/filter.hpp"
#include "bookmark_service/mtry.h"
#include "bookmark_service/receiver.hpp"
#include "bookmark_service/service.hpp"
#include "bookmark_service/transform.hpp"
#include "bookmark_service/trim.hpp"
#include "bookmark_service/value.hpp"
#include "nlohmann/json.hpp"

using json = nlohmann::json;
using expected_json = bms::expected<json, std::exception_ptr>;

struct bookmark_t {
  std::string url;
  std::string text;
};

std::string to_string(const bookmark_t& page) {
  std::string temp{};
  temp.reserve(page.text.size() + page.url.size() + 10);
  temp.append("[").append(page.text).append("](").append(page.url).append(")");
  return temp;
}
std::ostream& operator<<(std::ostream& os, const bookmark_t& page) {
  os << to_string(page);
  return os;
}

using expected_bookmark = bms::expected<bookmark_t, std::exception_ptr>;

expected_bookmark bookmark_from_json(const json& data) {
  return bms::mtry([&] {
    return bookmark_t{data.at("FirstURL"), data.at("Text")};
  });
}

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
