#include <array>
#include <asio.hpp>
#include <exception>
#include <iostream>
#include <memory>
#include <system_error>

using asio::buffer;
using asio::ip::tcp;

class proxy : public std::enable_shared_from_this<proxy> {
 public:
  explicit proxy(tcp::socket client)
      : client_(std::move(client)), server_(client_.get_executor()) {}

  void connect_to_server(tcp::endpoint target) {
    auto self = shared_from_this();
    server_.async_connect(target, [self](std::error_code ec) {
      if (!ec) {
        self->read_from_client();
        self->read_from_server();
      }
    });
  }

 private:
  void stop() {
    client_.close();
    server_.close();
  }
  void read_from_client() {
    auto self = shared_from_this();
    client_.async_read_some(buffer(data_from_client_),
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
    asio::async_write(server_, buffer(data_from_client_, n),
                      [self](std::error_code ec, std::size_t n) {
                        if (!ec) {
                          self->read_from_client();
                        } else {
                          self->stop();
                        }
                      });
  }
  void read_from_server() {
    auto self = shared_from_this();
    server_.async_read_some(buffer(data_from_server_),
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
    asio::async_write(client_, buffer(data_from_server_, n),
                      [self](std::error_code ec, size_t n) {
                        if (!ec) {
                          self->read_from_server();
                        } else {
                          self->stop();
                        }
                      });
  }

 private:
  tcp::socket client_;
  tcp::socket server_;
  std::array<char, 1024> data_from_client_;
  std::array<char, 1024> data_from_server_;
};

void listen(tcp::acceptor& acceptor, tcp::endpoint target) {
  acceptor.async_accept(
      [&acceptor, target](std::error_code ec, tcp::socket client) {
        if (!ec) {
          std::make_shared<proxy>(std::move(client))->connect_to_server(target);
        }
        listen(acceptor, target);
      });
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
    listen(acceptor, target_endpoint);
    ctx.run();

  } catch (std::exception& e) {
    std::cerr << "Exception: " << e.what() << "\n";
  }
}