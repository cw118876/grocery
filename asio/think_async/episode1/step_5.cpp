#include <array>
#include <chrono>
#include <exception>
#include <iostream>
#include <memory>
#include <system_error>
#include "asio.hpp"
#include "asio/awaitable.hpp"
#include "asio/co_spawn.hpp"
#include "asio/detached.hpp"
#include "asio/experimental/as_tuple.hpp"
#include "asio/experimental/awaitable_operators.hpp"
#include "asio/steady_timer.hpp"
#include "asio/this_coro.hpp"
#include "asio/use_awaitable.hpp"

using asio::awaitable;
using asio::buffer;
using asio::co_spawn;
using asio::detached;
using asio::use_awaitable;
using asio::ip::tcp;
using std::chrono::steady_clock;
using namespace std::chrono_literals;
using namespace asio::experimental::awaitable_operators;

constexpr auto use_nothrow_awaitable =
    asio::experimental::as_tuple(asio::use_awaitable);

awaitable<void> transfer(tcp::socket& from,
                         tcp::socket& to,
                         steady_clock::time_point& deadline) {
  std::array<char, 1024> data;
  for (;;) {
    deadline = std::max(deadline, std::chrono::steady_clock::now() + 10s);
    auto [e1, n1] =
        co_await from.async_read_some(buffer(data), use_nothrow_awaitable);
    if (e1) {
      co_return;
    }
    auto [e2, n2] =
        co_await asio::async_write(to, buffer(data, n1), use_nothrow_awaitable);
    if (e2) {
      co_return;
    }
  }
}

awaitable<void> watchdog(steady_clock::time_point& deadline) {
  asio::steady_timer timer(co_await asio::this_coro::executor);
  auto now = steady_clock::now();
  while (deadline > now) {
    timer.expires_at(deadline);
    co_await timer.async_wait(use_nothrow_awaitable);
    now = steady_clock::now();
  }
}

awaitable<void> proxy(tcp::socket client, tcp::endpoint target) {
  tcp::socket server(client.get_executor());
  steady_clock::time_point deadline{};
  auto [e] = co_await server.async_connect(target, use_nothrow_awaitable);
  if (!e) {
    co_await (transfer(server, client, deadline) ||
              transfer(client, server, deadline) || watchdog(deadline)

    );
    client.close();
    server.close();
  }
}

awaitable<void> listen(tcp::acceptor& acceptor, tcp::endpoint target) {
  for (;;) {
    auto [e, client] = co_await acceptor.async_accept(use_nothrow_awaitable);
    if (e) {
      break;
    }
    auto ex = client.get_executor();
    co_spawn(ex, proxy(std::move(client), target), detached);
  }
}

int main(int argc, char* argv[]) {
  try {
    if (argc != 5) {
      std::cerr << "Usage: proxy";
      std::cerr << " <listen address> <listen port>";
      std::cerr << " <target address> <target_port>";
      return 1;
    }
    asio::io_context ctx;
    auto listen_endpoint =
        *tcp::resolver(ctx).resolve(argv[1], argv[2], tcp::resolver::passive);
    auto target_endpoint = *tcp::resolver(ctx).resolve(argv[3], argv[4]);
    tcp::acceptor acceptor(ctx, listen_endpoint);
    co_spawn(ctx, listen(acceptor, target_endpoint), detached);
    ctx.run();

  } catch (std::exception& e) {
    std::cerr << "Exception: " << e.what() << "\n";
  }
}