//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++03

// <functional>

// class function<R(ArgTypes...)>

// function& operator=(const function& f);

#include <cassert>
#include <utility>

#include "test_macros.h"
#include "count_new.h"

#include "function/function.hpp"
#include "gtest/gtest.h"

namespace {
class A {
  int data_[10];

public:
  inline static int count = 0;

  A() {
    ++count;
    for (int i = 0; i < 10; ++i)
      data_[i] = i;
  }

  A(const A &) { ++count; }

  ~A() { --count; }

  int operator()(int i) const {
    for (int j = 0; j < 10; ++j)
      i += data_[j];
    return i;
  }
};

int g0() { return 0; }
int g(int) { return 0; }
int g2(int, int) { return 2; }
int g3(int, int, int) { return 3; }

}

TEST(FunctionFuncCommon, copyAssign) {
  globalMemCounter.reset();
  EXPECT_EQ(globalMemCounter.checkOutstandingNewEq(0), true);
  {
    tmp::function<int(int)> f = A();
    EXPECT_EQ(A::count, 1);
    EXPECT_EQ(globalMemCounter.checkOutstandingNewLessThanOrEqual(1), true);
    tmp::function<int(int)> f2;
    f2 = f;
    EXPECT_EQ(A::count, 2);
    EXPECT_EQ(globalMemCounter.checkOutstandingNewLessThanOrEqual(2), true);
  }
  EXPECT_EQ(A::count, 0);
  EXPECT_TRUE(globalMemCounter.checkOutstandingNewEq(0));
  {
    tmp::function<int(int)> f = g;
    EXPECT_TRUE(globalMemCounter.checkOutstandingNewEq(0));
    tmp::function<int(int)> f2;
    f2 = f;
    EXPECT_TRUE(globalMemCounter.checkOutstandingNewEq(0));
  }
  EXPECT_TRUE(globalMemCounter.checkOutstandingNewEq(0));
  {
    tmp::function<int(int)> f;
    EXPECT_TRUE(globalMemCounter.checkOutstandingNewEq(0));
    tmp::function<int(int)> f2;
    f2 = f;
    EXPECT_TRUE(globalMemCounter.checkOutstandingNewEq(0));
  }
  {
    typedef tmp::function<int()> Func;
    Func f = g0;
    Func& fr = (f = (Func &)f);
    EXPECT_TRUE(&fr == &f);
  }
  {
    typedef tmp::function<int(int)> Func;
    Func f = g;
    Func& fr = (f = (Func &)f);
    EXPECT_TRUE(&fr == &f);
  }
  {
    typedef tmp::function<int(int, int)> Func;
    Func f = g2;
    Func& fr = (f = (Func &)f);
    EXPECT_TRUE(&fr == &f);
  }
  {
    typedef tmp::function<int(int, int, int)> Func;
    Func f = g3;
    Func& fr = (f = (Func &)f);
    EXPECT_TRUE(&fr == &f);
  }
#if TEST_STD_VER >= 11
  EXPECT_TRUE(globalMemCounter.checkOutstandingNewEq(0));
  {
    tmp::function<int(int)> f = A();
    EXPECT_TRUE(A::count == 1);
    EXPECT_TRUE(globalMemCounter.checkOutstandingNewLessThanOrEqual(1));
    tmp::function<int(int)> f2;
    f2 = std::move(f);
    EXPECT_TRUE(A::count == 1);
    EXPECT_TRUE(globalMemCounter.checkOutstandingNewLessThanOrEqual(1));
#endif
}

}