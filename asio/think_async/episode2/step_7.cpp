#include <chrono>
#include <iostream>
#include <string>
#include "asio.hpp"
#include "asio/awaitable.hpp"
#include "asio/buffer.hpp"
#include "asio/co_spawn.hpp"
#include "asio/detached.hpp"
#include "asio/experimental/as_tuple.hpp"
#include "asio/experimental/awaitable_operators.hpp"
#include "asio/io_context.hpp"
#include "asio/use_awaitable.hpp"

using asio::awaitable;
using asio::co_spawn;
using asio::detached;
using asio::dynamic_buffer;
using asio::ip::tcp;
namespace this_coro = asio::this_coro;
using namespace asio::experimental::awaitable_operators;
using std::chrono::steady_clock;
using namespace std::chrono_literals;

constexpr auto use_nothrow_awaitable =
    asio::experimental::as_tuple(asio::use_awaitable);

template <typename Stream>
class MessageReader {
 public:
  explicit MessageReader(Stream& stream) : stream_(stream) {}
  awaitable<std::string> readMessage() {
    auto [e, n] = co_await asio::async_read_until(
        stream_, dynamic_buffer(messageBuf_), '|', use_nothrow_awaitable);
    if (e) {
      co_return std::string{};
    }
    std::string message(messageBuf_.substr(0, n));
    messageBuf_.erase(0, n);
    co_return message;
  }

 private:
  Stream& stream_;
  std::string messageBuf_;
};

awaitable<void> session(tcp::socket client) {
  MessageReader<tcp::socket> reader(client);

  for (;;) {
    std::string message = co_await reader.readMessage();
    if (!message.empty()) {
      std::cout << "received: " << message << "\n";
    } else {
      co_return;
    }
  }
}

awaitable<void> listen(tcp::acceptor& accptor) {
  for (;;) {
    auto [e, client] = co_await accptor.async_accept(use_nothrow_awaitable);
    if (e) {
      co_return;
    }
    auto ex = client.get_executor();
    co_spawn(ex, session(std::move(client)), detached);
  }
}

int main(int argc, char* argv[]) {
  try {
    if (argc != 3) {
      std::cerr << "Usage: message server: ";
      std::cerr << "<listen_address> <listen_port>\n";
      return 1;
    }
    asio::io_context ctx;
    auto listenEndPoint =
        *tcp::resolver(ctx).resolve(argv[1], argv[2], tcp::resolver::passive);
    tcp::acceptor acceptor(ctx, listenEndPoint);
    co_spawn(ctx, listen(acceptor), detached);
    ctx.run();
  } catch (std::exception& e) {
    std::cerr << "Exception: " << e.what() << "\n";
    return 1;
  }
}