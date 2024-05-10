#include <array>
#include <iostream>
#include <memory>

#include "asio.hpp"
#include "asio/io_context.hpp"
#include "asio/write.hpp"

using asio::buffer;
using asio::ip::tcp;

class Proxy : public std::enable_shared_from_this<Proxy> {
 public:
  explicit Proxy(tcp::socket client)
      : client_(std::move(client)), server_(client_.get_executor()) {}
  void connect_to_server(tcp::endpoint target) {
    auto self = shared_from_this();
    self->server_.async_connect(target, [self](std::error_code ec) {
      if (!ec) {
        self->read_from_server();
        self->read_from_client();
      }
    });
  }

 private:
  void stop() {
    client_.close();
    server_.close();
  }
  void read_from_server() {
    auto self = shared_from_this();
    self->server_.async_read_some(buffer(serverToClientBuff_),
                                  [self](std::error_code ec, size_t n) {
                                    if (!ec) {
                                      self->write_to_client(n);
                                    } else {
                                      self->stop();
                                    }
                                  });
  }
  void write_to_client(size_t n) {
    auto self = shared_from_this();
    asio::async_write(self->client_, buffer(serverToClientBuff_, n),
                      [self](std::error_code ec, size_t n) {
                        if (!ec) {
                          self->read_from_server();
                        } else {
                          self->stop();
                        }
                      });
  }
  void read_from_client() {
    auto self = shared_from_this();
    self->client_.async_read_some(buffer(clientToServerBuff_),
                                  [self](std::error_code ec, size_t n) {
                                    if (!ec) {
                                      self->write_to_server(n);
                                    } else {
                                      self->stop();
                                    }
                                  });
  }

  void write_to_server(size_t n) {
    auto self = shared_from_this();
    asio::async_write(self->server_, buffer(clientToServerBuff_, n),
                      [self](std::error_code ec, size_t n) {
                        if (!ec) {
                          self->read_from_client();
                        } else {
                          self->stop();
                        }
                      });
  }

 private:
  tcp::socket client_;
  tcp::socket server_;
  std::array<char, 1024> clientToServerBuff_;
  std::array<char, 1024> serverToClientBuff_;
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