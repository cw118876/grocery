#include <iostream>
#include <new>
#include <memory>
#include "benchmark/benchmark.h"


template <typename T, typename Deleter>
class SmartPtr {
public:
 SmartPtr(T *ptr, Deleter del): ptr_{ptr}, del_{del}{}
 ~SmartPtr() {del_(ptr_);}
 T* operator*() { return ptr_;}
 const T* operator*() const { return ptr_;}


private:
 T* ptr_;
 Deleter del_;
};

template <typename T>
class SmartPtrTE {
 struct DeleterBase {
    virtual void apply(void*) = 0;
    virtual ~DeleterBase() {}
 };
 template <typename D>
 struct Deleter : public DeleterBase {
    Deleter(D d): del_{d} {}
    void apply(void *d) override {
        del_(static_cast<D*>(d));
    }
    D del_;
 };
 public:
 template <typename D>
 SmartPtrTE(T* t, D d): ptr_{t}, del_{ new Deleter<D> {d}}{}
 ~SmartPtrTE() {
    del_->apply(ptr_);
    delete del_;
 }
 T* operator*() { return ptr_;}
 const T* operator*() const { return ptr_;}
 private:
 T* ptr_;
 DeleterBase* del_;

};

template <typename T>
class SmartPtrTESSO  {
 public:
 struct DeleterBase {
   virtual void apply(void *d) = 0;
   virtual ~DeleterBase() {}
 };

 template <typename D>
 class Deleter: public DeleterBase {
   public:
    Deleter(D d): del_{d} {}
    void apply(void *d) override {
      del_(static_cast<D*>(d));
    }

   private:
    D del_;
 };
 template <typename D>
 SmartPtrTESSO(T *p, D d): ptr_{p} {
   if constexpr (sizeof(T) > kBufLength) {
      del_ = new Deleter<D> {d};
   } else {
      del_ = new (buf_) Deleter<D> {d};
   }
 }
 ~SmartPtrTESSO() {
   del_->apply(ptr_);
   if constexpr (sizeof(T) > kBufLength) {
      delete del_;
   } else {
      del_->~DeleterBase();
   }
 }
 T* operator*() { return ptr_;}
 const T* operator*() const { return ptr_;}

 private:
  constexpr static size_t kBufLength = 16;
  T* ptr_;
  DeleterBase* del_;
  char buf_[kBufLength];
};


struct Deleter {
   template <typename T>
   void operator()(T* p) {
      delete p;
   }
};

template <typename T>
struct DeleterT {
   void operator()(T* p) {
      delete p;
   }

};

void BM_rawptr(benchmark::State& state) {
   Deleter d;
   for (auto _: state) {
      int* p = new int {12};
      d(p);
   }
   state.SetItemsProcessed(state.iterations());
}

void BM_unique_ptr_default(benchmark::State& state) {
   for (auto _: state) {
      auto p = std::make_unique<int>(123);
   }
   state.SetItemsProcessed(state.iterations());
}

void BM_unique_ptr(benchmark::State& state) {
   Deleter d;
   for (auto _: state) {
      std::unique_ptr<int, Deleter> p {new int {12}, d};
   }
   state.SetItemsProcessed(state.iterations());
}

void BM_shared_ptr(benchmark::State& state) {
   for (auto _: state) {
      std::shared_ptr<int> p (new int {12});
   }
   state.SetItemsProcessed(state.iterations());
}

void BM_shared_ptr_make(benchmark::State& state) {
   for (auto _: state) {
      auto p = std::make_shared<int>(123);
   }
   state.SetItemsProcessed(state.iterations());
}

void BM_shared_ptr_deleter(benchmark::State& state) {
   Deleter d;
   for (auto _: state) {
      std::shared_ptr<int> p { new int {12}, d};
   }
   state.SetItemsProcessed(state.iterations());
}

void BM_smartptr(benchmark::State& state) {
   Deleter d;
   for (auto _: state) {
      SmartPtr<int, Deleter> p {new int{123}, d};
   }
   state.SetItemsProcessed(state.iterations());
}

void BM_smartptr_te(benchmark::State& state) {
   Deleter d;
   for (auto _: state) {
      SmartPtrTE<int> p {new int{123}, d};
   }
   state.SetItemsProcessed(state.iterations());
}

void BM_smartptr_te_sso(benchmark::State& state) {
   Deleter d;
   for (auto _: state) {
      SmartPtrTESSO<int> p {new int {123}, d};
   }
   state.SetItemsProcessed(state.iterations());
}

BENCHMARK(BM_rawptr);
BENCHMARK(BM_shared_ptr);
BENCHMARK(BM_shared_ptr_deleter);
BENCHMARK(BM_shared_ptr_make);
BENCHMARK(BM_unique_ptr);
BENCHMARK(BM_unique_ptr_default);
BENCHMARK(BM_smartptr);
BENCHMARK(BM_smartptr_te);
BENCHMARK(BM_smartptr_te_sso);

BENCHMARK_MAIN();