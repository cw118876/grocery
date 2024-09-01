#ifndef BOOKMARK_SERVICE_SESSION_HPP_
#define BOOKMARK_SERVICE_SESSION_HPP_

#include <memory>

#include "asio.hpp"

namespace bms {

template <typename MsgHandler>
class session : public std::enable_shared_from_this<session<MsgHandler>> {
 public:
  session(asio::ip::tcp::socket&& sock, MsgHandler h)
      : socket_{std::move(sock)}, handler_(h) {}

  void start() { do_read(); }

 private:
  using shared_base_type = std::enable_shared_from_this<session<MsgHandler>>;
  void do_read() {
    auto self = shared_base_type::shared_from_this();
    asio::async_read_until(
        socket_, buf_, "\n",
        [self](const asio::error_code& ec, std::size_t size) {
          if (!ec) {
            std::istream is(&self->buf_);
            std::string line;
            (void)size;
            std::getline(is, line);
            self->handler_(std::move(line));
            self->do_read();
          }
        });
    // socket_.rea
  }
  asio::ip::tcp::socket socket_;
  asio::streambuf buf_;
  MsgHandler handler_;
};

template <typename MsgHandler>
auto make_shared_session(asio::ip::tcp::socket&& sock, MsgHandler h)
    -> std::shared_ptr<session<MsgHandler>> {
  return std::make_shared<session<MsgHandler>>(std::move(sock), h);
}

}  // namespace bms

#endif  // #ifndef BOOKMARK_SERVICE_ACTOR_HPP_