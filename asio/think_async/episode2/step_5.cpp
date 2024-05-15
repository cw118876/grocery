#include <chrono>
#include <csignal>
#include <iostream>
#include <string>
#include <system_error>
#include "asio.hpp"
#include "asio/associated_allocator.hpp"
#include "asio/associated_executor.hpp"
#include "asio/async_result.hpp"
#include "asio/awaitable.hpp"
#include "asio/bind_allocator.hpp"
#include "asio/bind_executor.hpp"
#include "asio/detached.hpp"
#include "asio/experimental/awaitable_operators.hpp"
#include "asio/io_context.hpp"
#include "asio/signal_set.hpp"
#include "asio/steady_timer.hpp"
#include "asio/this_coro.hpp"
#include "asio/use_awaitable.hpp"

using asio::awaitable;
using asio::buffer;
using asio::co_spawn;
using asio::detached;
using asio::use_awaitable;

namespace this_coro = asio::this_coro;

using namespace asio::experimental::awaitable_operators;
using std::chrono::steady_clock;
using namespace std::chrono_literals;

template <class CompletionToken>
auto async_wait_for_signal(asio::signal_set& sigset, CompletionToken&& token) {
  return asio::async_initiate<CompletionToken,
                              void(std::error_code, std::string)>(
      [&sigset](auto handler) {
        auto executator =
            asio::get_associated_executor(handler, sigset.get_executor());
        auto intermediate_handler = [handler = std::move(handler)](
                                        std::error_code ec, int signo) mutable {
          std::string signame;
          switch (signo) {
            case SIGABRT:
              signame = "SIGABRT";
              break;
            case SIGFPE:
              signame = "SIGFPE";
              break;
            case SIGILL:
              signame = "SIGLL";
              break;
            case SIGINT:
              signame = "SIGINT";
              break;
            case SIGSEGV:
              signame = "SIGSEGV";
              break;
            case SIGTERM:
              signame = "SIGTERM";
              break;
            default:
              signame = "<other>";
              break;
          }
          std::move(handler)(ec, signame);
        };
        sigset.async_wait(
            asio::bind_executor(executator, std::move(intermediate_handler)));
      },
      token);
}

awaitable<void> timed_wait_for_signal() {
  asio::signal_set sigset(co_await this_coro::executor, SIGINT, SIGTERM);
  asio::steady_timer timer(co_await this_coro::executor, 5s);
  auto result = co_await (async_wait_for_signal(sigset, use_awaitable) ||
                          timer.async_wait(use_awaitable));
  switch (result.index()) {
    case 0:
      std::cout << "Signal finished first: " << std::get<0>(result) << "\n";
      break;
    case 1:
      std::cout << "timer finished first\n";
      break;
  }
}

int main() {
  asio::io_context ctx;
  co_spawn(ctx, timed_wait_for_signal(), detached);
  ctx.run();
}