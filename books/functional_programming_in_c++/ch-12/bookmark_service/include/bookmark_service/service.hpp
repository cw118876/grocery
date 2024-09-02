#ifndef BOOKMARK_SERVICE_SERVICE_HPP_
#define BOOKMARK_SERVICE_SERVICE_HPP_

#include <string>
#include "asio.hpp"
#include "bookmark_service/actor.hpp"
#include "bookmark_service/session.hpp"

namespace bms {

class service : private source<std::string> {
 private:
  using base_type = source<std::string>;

 public:
  // using base_type::on_out_message;
  using base_type::set_out_handler;
  using base_type::out_type;
  using base_type::out_handler_t;;
  
  service() = delete;
  service(const service&) = delete;
  service& operator=(const service&) = delete;
  // just for simplicity,  maybe support later.
  service(service&& other) = delete;
  service& operator=(service&&) = delete;

  ~service() = default;
  explicit service(asio::io_context& io, uint16_t port = 45678)
      : acceptor_(io, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port)),
        socket_(io) {}

  void start() { do_accept(); }

 private:
  void do_accept() {
    acceptor_.async_accept(socket_, [this](const asio::error_code& ec) {
      if (!ec) {
        auto session =
            make_shared_session(std::move(socket_), get_out_handler());
        session->start();
      }
      do_accept();
    });
  }
  asio::ip::tcp::acceptor acceptor_;
  asio::ip::tcp::socket socket_;
};

}  // namespace bms

#endif  // BOOKMARK_SERVICE_SERVICE_HPP_
