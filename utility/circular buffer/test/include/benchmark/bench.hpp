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
    ::cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(cpu, &cpuset);
    if (::pthread_setaffinity_np(::pthread_self(), sizeof(cpuset), &cpuset)) {
        std::perror("pthread_setaffinity_np");
        std::exit(EXIT_FAILURE);
    }
}

template <class Tp>
struct isRigtorp: std::false_type {};


template <class Tp>
class Bench {
public:
 using value_type = typename Tp::value_type;
 static constexpr size_t kFifoSize = 131072;

private:

};

#endif  // BENCHMARK_BENCH_HPP_