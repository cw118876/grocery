#include <array>
#include <chrono>
#include <iostream>
#include <memory>
#include <system_error>

#include "asio.hpp"
#include "asio/bind_cancellation_slot.hpp"
#include "asio/buffer.hpp"
#include "asio/cancellation_signal.hpp"
#include "asio/completion_condition.hpp"
#include "asio/error.hpp"
#include "asio/io_context.hpp"
#include "asio/steady_timer.hpp"
#include "asio/write.hpp"

using asio::buffer;
using asio::ip::tcp;
using std::chrono::steady_clock;
using namespace std::chrono_literals;

class Proxy : public std::enable_shared_from_this<Proxy> {
 public:
  explicit Proxy(tcp::socket client)
      : client_(std::move(client)),
        server_(client_.get_executor()),
        watchdog_timer_(client_.get_executor()),
        heartbeat_timer_(client_.get_executor()) {}
  void connect_to_server(tcp::endpoint target) {
    auto self = shared_from_this();
    self->server_.async_connect(target, [self](std::error_code ec) {
      if (!ec) {
        self->read_from_server();
        self->read_from_client();
        self->watchdog();
        self->heartbeat();
      }
    });
  }

 private:
  void stop() {
    client_.cancel();
    server_.cancel();
    watchdog_timer_.cancel();
  }
  bool is_stopped() const { return !client_.is_open() && !server_.is_open(); }

  void watchdog() {
    auto self = shared_from_this();
    watchdog_timer_.expires_at(deadline_);
    watchdog_timer_.async_wait([self](std::error_code ec) {
      if (!self->is_stopped()) {
        auto now = steady_clock::now();
        if (self->deadline_ > now) {
          self->watchdog();
        } else {
          self->stop();
        }
      }
    });
  }
  void read_from_server() {
    deadline_ = std::max(deadline_, steady_clock::now() + 10s);
    auto self = shared_from_this();
    self->server_.async_read_some(
        buffer(serverToClientBuff_),
        asio::bind_cancellation_slot(
            heartbeat_signal_.slot(), [self](std::error_code ec, size_t n) {
              if (!ec) {
                self->num_hearbeats_ = 0;
                self->write_to_client(n);
              } else if (ec == asio::error::operation_aborted) {
                ++self->num_hearbeats_;
                self->write_heartbeat_to_client();
              } else {
                self->stop();
              }
            }));
  }
  void write_to_client(size_t n) {
    auto self = shared_from_this();
    asio::async_write(
        self->client_, buffer(serverToClientBuff_, n),
        [self](std::error_code ec, size_t n) {
          if (!ec) {
            self->read_from_server();
          } else {
            self->stop();
          }
        });
  }
  void read_from_client() {
    deadline_ = std::max(deadline_, steady_clock::now() + 10s);
    auto self = shared_from_this();
    self->client_.async_read_some(buffer(clientToServerBuff_),
                                  [self](std::error_code ec, size_t n) {
                                    if (!ec && !self->is_stopped()) {
                                      self->write_to_server(n);
                                    } else {
                                      self->stop();
                                    }
                                  });
  }

  void write_to_server(size_t n) {
    auto self = shared_from_this();
    asio::async_write(
        self->server_, buffer(clientToServerBuff_, n),
        [self](std::error_code ec, size_t n) {
          if (!ec) {
            self->read_from_client();
          } else {
            self->stop();
          }
        });
  }

  void write_heartbeat_to_client() {
    size_t n = asio::buffer_copy(
        buffer(serverToClientBuff_),
        std::array<asio::const_buffer, 3>{
            buffer("<heartbeat "), buffer(std::to_string(num_hearbeats_)),
            buffer(">\r\n")});
    write_to_client(n);
  }

  void heartbeat() {
    auto self = shared_from_this();
    heartbeat_timer_.expires_after(1s);
    heartbeat_timer_.async_wait([self](std::error_code) {
      if (!self->is_stopped()) {
        self->heartbeat_signal_.emit(asio::cancellation_type::total);
        self->heartbeat();
      }
    });
  }

 private:
  tcp::socket client_;
  tcp::socket server_;
  std::array<char, 1024> clientToServerBuff_;
  std::array<char, 1024> serverToClientBuff_;
  steady_clock::time_point deadline_;
  asio::steady_timer watchdog_timer_;
  asio::steady_timer heartbeat_timer_;
  asio::cancellation_signal heartbeat_signal_;
  size_t num_hearbeats_ = 0;
};

void listen(tcp::acceptor& acceptor, tcp::endpoint target) {
  acceptor.async_accept(target, [&acceptor, target](std::error_code ec,
                                                    tcp::socket client) {
    if (!ec) {
      std::shared_ptr<Proxy> proxy = std::make_shared<Proxy>(std::move(client));
      proxy->connect_to_server(target);
    }
    listen(acceptor, target);
  });
}

int main(int argc, const char* argv[]) {
  try {
    if (argc != 5) {
      std::cerr << " Usage: proxy ";
      std::cerr << "<listen address> <listen port> ";
      std::cerr << "<target_address> <target_port>\n";
      return 1;
    }
    asio::io_context ctx;
    auto listen_endpoint =
        *tcp::resolver(ctx).resolve(argv[1], argv[2], tcp::resolver::passive);
    auto target_endpoint = *tcp::resolver(ctx).resolve(argv[3], argv[4]);
    tcp::acceptor acceptor(ctx, listen_endpoint);

    listen(acceptor, target_endpoint);
    ctx.run();

  } catch (std::exception& e) {
    std::cerr << "Exception: " << e.what() << "\n";
  }
  return 0;
}