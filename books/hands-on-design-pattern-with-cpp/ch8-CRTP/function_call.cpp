#include <cstdlib>
#include <memory>

#include "benchmark/benchmark.h"

#define REPEATE2(x) x x
#define REPEATE4(x) REPEATE2(x) REPEATE2(x)
#define REPEATE8(x) REPEATE4(x) REPEATE4(x)
#define REPEATE16(x) REPEATE8(x) REPEATE8(x)
#define REPEATE32(x) REPEATE16(x) REPEATE16(x)
#define REPEATE(x) REPEATE32(x)

namespace no_polymorphism {

class A {
 public:
  void f(int i) noexcept { i_ += i; }
  int get() const noexcept { return i_; }

 private:
  int i_ = 0;
};

}  // namespace no_polymorphism

namespace dynamic_polymorphism {
class B {
 public:
  virtual ~B() {}
  virtual void f(int i) = 0;
  int get() const noexcept { return i_; }

 protected:
  int i_ = 0;
};

class D : public B {
  void f(int i) override { i_ += i; }
};

}  // namespace dynamic_polymorphism

namespace static_polymorphism {
template <typename D>
class B {
 public:
  // virtual destructor or not ?
  void f(int i) { return static_cast<D*>(this)->f(i); }
  int get() const noexcept { return i_; }

 protected:
  int i_ = 0;
};

class D : public B<D> {
 public:
  void f(int i) { i_ += i; }
};

template <typename T>
void apply(B<T>* b, int& i) {
  b->f(++i);
}

}  // namespace static_polymorphism

void BM_non(benchmark::State& state) {
  std::unique_ptr<no_polymorphism::A> ptr{new no_polymorphism::A{}};
  int i = 0;
  // remove the effect of unique_ptr
  no_polymorphism::A* p = ptr.get();
  for (auto _ : state) {
    REPEATE(p->f(++i););
  }
  benchmark::DoNotOptimize(p->get());
  state.SetItemsProcessed(32 * state.iterations());
}

void apply(dynamic_polymorphism::B* ptr, int i) {
  ptr->f(i);
}

void BM_dynamic(benchmark::State& state) {
  std::unique_ptr<dynamic_polymorphism::B> ptr{new dynamic_polymorphism::D{}};
  int i = 0;
  // remove the effect of unique_ptr
  dynamic_polymorphism::B* p = ptr.get();
  for (auto _ : state) {
    REPEATE(apply(p, ++i););
  }
  benchmark::DoNotOptimize(p->get());
  state.SetItemsProcessed(32 * state.iterations());
}

void BM_static(benchmark::State& state) {
  using value_type = static_polymorphism::B<static_polymorphism::D>;
  std::unique_ptr<value_type> ptr{new static_polymorphism::D{}};
  int i = 0;
  value_type* p = ptr.get();
  for (auto _ : state) {
    REPEATE(apply(p, ++i););
  }
  benchmark::DoNotOptimize(p->get());
  state.SetItemsProcessed(32 * state.iterations());
}

BENCHMARK(BM_non);
BENCHMARK(BM_static);
BENCHMARK(BM_dynamic);

BENCHMARK_MAIN();