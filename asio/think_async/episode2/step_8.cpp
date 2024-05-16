#include <chrono>
#include <ctime>
#include <iostream>
#include <string>
#include "asio.hpp"
#include "asio/awaitable.hpp"
#include "asio/buffer.hpp"
#include "asio/cancellation_signal.hpp"
#include "asio/cancellation_type.hpp"
#include "asio/co_spawn.hpp"
#include "asio/detached.hpp"
#include "asio/experimental/as_tuple.hpp"
#include "asio/experimental/awaitable_operators.hpp"
#include "asio/io_context.hpp"
#include "asio/steady_timer.hpp"
#include "asio/this_coro.hpp"
#include "asio/use_awaitable.hpp"

using asio::awaitable;
using asio::cancellation_type;
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
    co_await this_coro::reset_cancellation_state(
        [](cancellation_type requested) {
          if ((requested & cancellation_type::total) !=
              cancellation_type::none) {
            return cancellation_type::partial;
          } else {
            return requested;
          }
        });
    auto [e, n] = co_await asio::async_read_until(
        stream_, dynamic_buffer(messageBuf_), '|', use_nothrow_awaitable);
    if ((co_await this_coro::cancellation_state).cancelled() !=
        cancellation_type::none) {
      co_return std::string{};
    }
    co_await this_coro::reset_cancellation_state();
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

awaitable<void> timeout(steady_clock::duration duration) {
  asio::steady_timer timer(co_await this_coro::executor);
  timer.expires_after(duration);
  co_await timer.async_wait(use_nothrow_awaitable);
}

awaitable<void> session(tcp::socket client) {
  MessageReader<tcp::socket> reader(client);

  for (;;) {
    auto result = co_await (reader.readMessage() || timeout(5s));
    switch (result.index()) {
      case 0: {
        if (!std::get<0>(result).empty()) {
          std::cout << "received: " << std::get<0>(result) << "\n";
        } else {
          co_return;
        }
      } break;
      case 1:
        std::cout << "timed out\n";
        break;
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