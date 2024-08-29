//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

// FIXME: In MSVC mode, even "std::function<int(int)> f(aref);" causes
// allocations.
// XFAIL: target=x86_64-pc-windows-msvc && stdlib=libc++ && libcpp-abi-version=1

// UNSUPPORTED: c++03

// <functional>

// class function<R(ArgTypes...)>

// function(const function&  f);
// function(function&& f); // noexcept in C++20

#include <cstdlib>
#include <memory>
#include <utility>

#include "count_new.h"
#include "test_macros.h"

#include "function/function.hpp"
#include "gtest/gtest.h"

namespace {
class A {
  int data_[10];

 public:
  static int count;

  A() {
    ++count;
    for (int i = 0; i < 10; ++i)
      data_[i] = i;
  }

  A(const A&) { ++count; }

  ~A() { --count; }

  int operator()(int i) const {
    for (int j = 0; j < 10; ++j)
      i += data_[j];
    return i;
  }
};

int A::count = 0;

int g(int) {
  return 0;
}

}  // namespace

// #undef RTTI_ASSERT
// #define RTTI_ASSERT(x) (void);

TEST(FunctionFuncCommon, copyMove) {
  globalMemCounter.reset();
  EXPECT_TRUE(globalMemCounter.checkOutstandingNewEq(0));
  {
    tmp::function<int(int)> f = A();
    EXPECT_TRUE(A::count == 1);
    EXPECT_TRUE(globalMemCounter.checkOutstandingNewLessThanOrEqual(1));
    RTTI_ASSERT(f.target<A>());
    RTTI_ASSERT(f.target<int (*)(int)>() == 0);
    tmp::function<int(int)> f2 = f;
    EXPECT_TRUE(A::count == 2);
    EXPECT_TRUE(globalMemCounter.checkOutstandingNewLessThanOrEqual(2));
    RTTI_ASSERT(f2.target<A>());
    RTTI_ASSERT(f2.target<int (*)(int)>() == 0);
  }
  EXPECT_TRUE(A::count == 0);
  EXPECT_TRUE(globalMemCounter.checkOutstandingNewEq(0));
  {
    tmp::function<int(int)> f = g;
    EXPECT_TRUE(globalMemCounter.checkOutstandingNewEq(0));
    RTTI_ASSERT(f.target<int (*)(int)>());
    RTTI_ASSERT(f.target<A>() == 0);
    tmp::function<int(int)> f2 = f;
    EXPECT_TRUE(globalMemCounter.checkOutstandingNewEq(0));
    RTTI_ASSERT(f2.target<int (*)(int)>());
    RTTI_ASSERT(f2.target<A>() == 0);
  }
  EXPECT_TRUE(globalMemCounter.checkOutstandingNewEq(0));
  {
    tmp::function<int(int)> f;
    EXPECT_TRUE(globalMemCounter.checkOutstandingNewEq(0));
    RTTI_ASSERT(f.target<int (*)(int)>() == 0);
    RTTI_ASSERT(f.target<A>() == 0);
    tmp::function<int(int)> f2 = f;
    EXPECT_TRUE(globalMemCounter.checkOutstandingNewEq(0));
    RTTI_ASSERT(f2.target<int (*)(int)>() == 0);
    RTTI_ASSERT(f2.target<A>() == 0);
  }
  {
    tmp::function<int(int)> f;
    EXPECT_TRUE(globalMemCounter.checkOutstandingNewEq(0));
    RTTI_ASSERT(f.target<int (*)(int)>() == 0);
    RTTI_ASSERT(f.target<A>() == 0);
    EXPECT_TRUE(!f);
    tmp::function<long(int)> g = f;
    EXPECT_TRUE(globalMemCounter.checkOutstandingNewEq(0));
    RTTI_ASSERT(g.target<long (*)(int)>() == 0);
    RTTI_ASSERT(g.target<A>() == 0);
    EXPECT_TRUE(!g);
  }
#if TEST_STD_VER >= 11
  EXPECT_TRUE(globalMemCounter.checkOutstandingNewEq(0));
  {  // Test rvalue references
    tmp::function<int(int)> f = A();
    EXPECT_TRUE(A::count == 1);
    EXPECT_TRUE(globalMemCounter.checkOutstandingNewLessThanOrEqual(1));
    RTTI_ASSERT(f.target<A>());
    RTTI_ASSERT(f.target<int (*)(int)>() == 0);
    LIBCPP_ASSERT_NOEXCEPT(tmp::function<int(int)>(std::move(f)));
#if TEST_STD_VER > 17
    ASSERT_NOEXCEPT(tmp::function<int(int)>(std::move(f)));
#endif
    tmp::function<int(int)> f2 = std::move(f);
    EXPECT_TRUE(A::count == 1);
    EXPECT_TRUE(globalMemCounter.checkOutstandingNewLessThanOrEqual(1));
    RTTI_ASSERT(f2.target<A>());
    RTTI_ASSERT(f2.target<int (*)(int)>() == 0);
    RTTI_ASSERT(f.target<A>() == 0);
    RTTI_ASSERT(f.target<int (*)(int)>() == 0);
  }
  EXPECT_TRUE(globalMemCounter.checkOutstandingNewEq(0));
  {
    // Test that moving a function constructed from a reference wrapper
    // is done without allocating.
    DisableAllocationGuard g;
    using Ref = std::reference_wrapper<A>;
    A a;
    Ref aref(a);
    tmp::function<int(int)> f(aref);
    EXPECT_TRUE(A::count == 1);
    RTTI_ASSERT(f.target<A>() == nullptr);
    RTTI_ASSERT(f.target<Ref>());
    LIBCPP_ASSERT_NOEXCEPT(tmp::function<int(int)>(std::move(f)));
#if TEST_STD_VER > 17
    ASSERT_NOEXCEPT(tmp::function<int(int)>(std::move(f)));
#endif
    tmp::function<int(int)> f2(std::move(f));
    EXPECT_TRUE(A::count == 1);
    RTTI_ASSERT(f2.target<A>() == nullptr);
    RTTI_ASSERT(f2.target<Ref>());
#if defined(_LIBCPP_VERSION)
    RTTI_ASSERT(f.target<Ref>());  // f is unchanged because the target is small
#endif
  }
  {
    // Test that moving a function constructed from a function pointer
    // is done without allocating
    DisableAllocationGuard guard;
    using Ptr = int (*)(int);
    Ptr p = g;
    tmp::function<int(int)> f(p);
    RTTI_ASSERT(f.target<A>() == nullptr);
    RTTI_ASSERT(f.target<Ptr>());
    LIBCPP_ASSERT_NOEXCEPT(tmp::function<int(int)>(std::move(f)));
#if TEST_STD_VER > 17
    ASSERT_NOEXCEPT(tmp::function<int(int)>(std::move(f)));
#endif
    tmp::function<int(int)> f2(std::move(f));
    RTTI_ASSERT(f2.target<A>() == nullptr);
    RTTI_ASSERT(f2.target<Ptr>());
#if defined(_LIBCPP_VERSION)
    RTTI_ASSERT(f.target<Ptr>());  // f is unchanged because the target is small
#endif
  }
#endif  // TEST_STD_VER >= 11
}
