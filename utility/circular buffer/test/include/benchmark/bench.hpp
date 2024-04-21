#ifndef BENCHMARK_BENCH_HPP_
#define BENCHMARK_BENCH_HPP_

#include <sched.h>
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <thread>
#include <utility>

#include <pthread.h>
#include <type_traits>

template <class Tp>
inline __attribute__((always_inline)) void doNotOptimize(const Tp& value) {
  asm volatile("" : : "r,m"(value) : "memory");
}

static void pinThread(int cpu) {
  if (cpu < 0) {
    return;
  }
  return;
  ::cpu_set_t cpuset;
  CPU_ZERO(&cpuset);
  CPU_SET(cpu, &cpuset);
  if (::pthread_setaffinity_np(::pthread_self(), sizeof(cpuset), &cpuset)) {
    std::perror("pthread_setaffinity_np");
    std::exit(EXIT_FAILURE);
  }
}

template <class Tp>
struct isRigtorp : std::false_type {};

template <class Tp>
class Bench {
 public:
  using value_type = typename Tp::value_type;
  static constexpr size_t kFifoSize = 131072;

  auto operator()(long iters, int cpu1, int cpu2) {
    using namespace std::chrono_literals;

    auto j = std::jthread([=, this] {
      pinThread(cpu1);
      // pop thread
      for (auto i = value_type{}; i < kFifoSize; ++i) {
        this->pop(i);
      }
      // pop the benchmark
      for (auto i = value_type{}; i < iters; ++i) {
        this->pop(i);
      }
    });
    pinThread(cpu2);
    // push warm
    for (auto i = value_type{}; i < kFifoSize; ++i) {
      push(i);
    }
    waitForEmpty();
    auto start = std::chrono::steady_clock::now();
    for (auto i = value_type{}; i < iters; ++i) {
      push(i);
    }
    waitForEmpty();
    auto end = std::chrono::steady_clock::now();
    auto duration = end - start;
    return (1s * iters) / duration;
  }

 private:
  void pop(value_type expected) {
    value_type value;
    if constexpr (isRigtorp<value_type>::value) {
      while (auto again = !queue_.front()) {
        doNotOptimize(again);
      }
      value = *queue_.front();
      queue_.pop();
    } else {
      while (auto again = !queue_.pop(value)) {
        doNotOptimize(again);
      }
    }
    if (value != expected) {
      throw std::runtime_error("invalid value");
    }
  }
  void push(value_type value) {
    if constexpr (isRigtorp<value_type>::value) {
      while (auto again = !queue_.try_push(value)) {
        doNotOptimize(again);
      }
    } else {
      while (auto again = !queue_.push(value)) {
        doNotOptimize(again);
      }
    }
  }
  void waitForEmpty() {
    while (auto again = !queue_.empty()) {
      doNotOptimize(again);
    }
  }
  Tp queue_{kFifoSize};
};

template <class Tp>
auto bench(const char* name, long iters, int cpu1, int cpu2) {
  return Bench<Tp>{}(iters, cpu1, cpu2);
}

template <template <class> class FifoT>
void bench(const char* name, int argc, const char* argv[]) {
  int cpu1 = 1;
  int cpu2 = 2;
  if (argc == 3) {
    cpu1 = std::atoi(argv[1]);
    cpu2 = std::atoi(argv[2]);
  }
  constexpr size_t iters = 100'000'000;
  using value_type = std::int64_t;
  auto opsPerSec = bench<FifoT<value_type>>(name, iters, cpu1, cpu2);
  std::cout << std::setw(7) << std::left << name << ":  " << std::setw(10)
            << std::right << opsPerSec << " ops/s\n";
}

#endif  // BENCHMARK_BENCH_HPP_