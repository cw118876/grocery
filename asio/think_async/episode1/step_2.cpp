#include <array>
#include <asio.hpp>
#include <exception>
#include <iostream>
#include <memory>
#include <system_error>
#include "asio/co_spawn.hpp"
#include "asio/detached.hpp"
#include "asio/use_awaitable.hpp"

#include "asio/experimental/as_tuple.hpp"

using asio::awaitable;
using asio::buffer;
using asio::co_spawn;
using asio::detached;
using asio::use_awaitable;
using asio::ip::tcp;

constexpr auto use_nothrow_awaitable =
    asio::experimental::as_tuple(asio::use_awaitable);

class proxy_state : public std::enable_shared_from_this<proxy_state> {
 public:
  explicit proxy_state(tcp::socket client)
      : client_(std::move(client)), server_(client_.get_executor()) {}
  void close() {
    client_.close();
    server_.close();
  }
  tcp::socket client_;
  tcp::socket server_;
};

using proxy_state_ptr = std::shared_ptr<proxy_state>;

awaitable<void> client_to_server(proxy_state_ptr state) {
  std::array<char, 1024> data;
  for (;;) {
    auto [e1, n1] = co_await state->client_.async_read_some(
        buffer(data), use_nothrow_awaitable);
    if (e1) {
      break;
    }
    auto [e2, n2] = co_await asio::async_write(state->server_, buffer(data, n1),
                                               use_nothrow_awaitable);
    if (e2) {
      break;
    }
  }
  state->close();
}

awaitable<void> server_to_client(proxy_state_ptr state) {
  std::array<char, 1024> data;
  for (;;) {
    auto [e1, n1] = co_await state->server_.async_read_some(
        buffer(data), use_nothrow_awaitable);
    if (e1) {
      break;
    }
    auto [e2, n2] = co_await asio::async_write(state->client_, buffer(data, n1),
                                               use_nothrow_awaitable);
    if (e2) {
      break;
    }
  }
  state->close();
}

awaitable<void> proxy(tcp::socket client, tcp::endpoint target) {
  auto state = std::make_shared<proxy_state>(std::move(client));
  auto [e] =
      co_await state->server_.async_connect(target, use_nothrow_awaitable);
  if (!e) {
    auto ex = state->client_.get_executor();
    co_spawn(ex, client_to_server(state), detached);
    co_await server_to_client(state);
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