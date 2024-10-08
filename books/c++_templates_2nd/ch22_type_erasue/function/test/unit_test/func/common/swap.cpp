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

// void swap(function& other);

#include "function/function.hpp"
#include "gtest/gtest.h"

#include "count_new.h"

#include "test_macros.h"

namespace {
class A {
  int data_[10];

public:
  static int count;

  explicit A(int j) {
    ++count;
    data_[0] = j;
  }

  A(const A &a) {
    ++count;
    for (int i = 0; i < 10; ++i)
      data_[i] = a.data_[i];
  }

  ~A() { --count; }

  int operator()(int i) const {
    for (int j = 0; j < 10; ++j)
      i += data_[j];
    return i;
  }

  int operator()() const { return -1; }
  int operator()(int, int) const { return -2; }
  int operator()(int, int, int) const { return -3; }

  int id() const { return data_[0]; }
};

int A::count = 0;

int g0() { return 0; }
int g(int) { return 0; }
int h(int) { return 1; }
int g2(int, int) { return 2; }
int g3(int, int, int) { return 3; }
}

TEST(FunctionFuncCommon, Swap) {
  globalMemCounter.reset();
  EXPECT_TRUE(globalMemCounter.checkOutstandingNewEq(0));
  {
    tmp::function<int(int)> f1 = A(1);
    tmp::function<int(int)> f2 = A(2);
    EXPECT_TRUE(A::count == 2);
    EXPECT_TRUE(globalMemCounter.checkOutstandingNewLessThanOrEqual(2));
    RTTI_ASSERT(f1.target<A>()->id() == 1);
    RTTI_ASSERT(f2.target<A>()->id() == 2);
    f1.swap(f2);
    EXPECT_TRUE(A::count == 2);
    EXPECT_TRUE(globalMemCounter.checkOutstandingNewLessThanOrEqual(2));
    RTTI_ASSERT(f1.target<A>()->id() == 2);
    RTTI_ASSERT(f2.target<A>()->id() == 1);
  }
  EXPECT_TRUE(A::count == 0);
  EXPECT_TRUE(globalMemCounter.checkOutstandingNewEq(0));
  {
    tmp::function<int(int)> f1 = A(1);
    tmp::function<int(int)> f2 = g;
    EXPECT_TRUE(A::count == 1);
    EXPECT_TRUE(globalMemCounter.checkOutstandingNewLessThanOrEqual(1));
    RTTI_ASSERT(f1.target<A>()->id() == 1);
    RTTI_ASSERT(*f2.target<int (*)(int)>() == g);
    f1.swap(f2);
    EXPECT_TRUE(A::count == 1);
    EXPECT_TRUE(globalMemCounter.checkOutstandingNewLessThanOrEqual(1));
    RTTI_ASSERT(*f1.target<int (*)(int)>() == g);
    RTTI_ASSERT(f2.target<A>()->id() == 1);
  }
  EXPECT_TRUE(A::count == 0);
  EXPECT_TRUE(globalMemCounter.checkOutstandingNewEq(0));
  {
    tmp::function<int(int)> f1 = g;
    tmp::function<int(int)> f2 = A(1);
    EXPECT_TRUE(A::count == 1);
    EXPECT_TRUE(globalMemCounter.checkOutstandingNewLessThanOrEqual(1));
    RTTI_ASSERT(*f1.target<int (*)(int)>() == g);
    RTTI_ASSERT(f2.target<A>()->id() == 1);
    f1.swap(f2);
    EXPECT_TRUE(A::count == 1);
    EXPECT_TRUE(globalMemCounter.checkOutstandingNewLessThanOrEqual(1));
    RTTI_ASSERT(f1.target<A>()->id() == 1);
    RTTI_ASSERT(*f2.target<int (*)(int)>() == g);
  }
  EXPECT_TRUE(A::count == 0);
  EXPECT_TRUE(globalMemCounter.checkOutstandingNewEq(0));
  {
    tmp::function<int(int)> f1 = g;
    tmp::function<int(int)> f2 = h;
    EXPECT_TRUE(A::count == 0);
    EXPECT_TRUE(globalMemCounter.checkOutstandingNewEq(0));
    RTTI_ASSERT(*f1.target<int (*)(int)>() == g);
    RTTI_ASSERT(*f2.target<int (*)(int)>() == h);
    f1.swap(f2);
    EXPECT_TRUE(A::count == 0);
    EXPECT_TRUE(globalMemCounter.checkOutstandingNewEq(0));
    RTTI_ASSERT(*f1.target<int (*)(int)>() == h);
    RTTI_ASSERT(*f2.target<int (*)(int)>() == g);
  }
  EXPECT_TRUE(A::count == 0);
  EXPECT_TRUE(globalMemCounter.checkOutstandingNewEq(0));
  {
    tmp::function<int(int)> f1 = A(1);
    EXPECT_TRUE(A::count == 1);
    {
      DisableAllocationGuard guard;
      ((void)guard);
      f1.swap(f1);
    }
    EXPECT_TRUE(A::count == 1);
    RTTI_ASSERT(f1.target<A>()->id() == 1);
  }
  EXPECT_TRUE(A::count == 0);
  EXPECT_TRUE(globalMemCounter.checkOutstandingNewEq(0));
  {
    tmp::function<int()> f1 = g0;
    DisableAllocationGuard guard;
    ((void)guard);
    f1.swap(f1);
    RTTI_ASSERT(*f1.target<int (*)()>() == g0);
  }
  EXPECT_TRUE(globalMemCounter.checkOutstandingNewEq(0));
  {
    tmp::function<int(int, int)> f1 = g2;
    DisableAllocationGuard guard;
    ((void)guard);
    f1.swap(f1);
    RTTI_ASSERT(*f1.target<int (*)(int, int)>() == g2);
  }
  EXPECT_TRUE(globalMemCounter.checkOutstandingNewEq(0));
  {
    tmp::function<int(int, int, int)> f1 = g3;
    DisableAllocationGuard guard;
    ((void)guard);
    f1.swap(f1);
    RTTI_ASSERT(*f1.target<int (*)(int, int, int)>() == g3);
  }
  EXPECT_TRUE(globalMemCounter.checkOutstandingNewEq(0));
  {
    tmp::function<int()> f1 = A(1);
    EXPECT_TRUE(A::count == 1);
    DisableAllocationGuard guard;
    ((void)guard);
    f1.swap(f1);
    EXPECT_TRUE(A::count == 1);
    RTTI_ASSERT(f1.target<A>()->id() == 1);
  }
  EXPECT_TRUE(globalMemCounter.checkOutstandingNewEq(0));
  EXPECT_TRUE(A::count == 0);
  {
    tmp::function<int(int, int)> f1 = A(2);
    EXPECT_TRUE(A::count == 1);
    DisableAllocationGuard guard;
    ((void)guard);
    f1.swap(f1);
    EXPECT_TRUE(A::count == 1);
    RTTI_ASSERT(f1.target<A>()->id() == 2);
  }
  EXPECT_TRUE(globalMemCounter.checkOutstandingNewEq(0));
  EXPECT_TRUE(A::count == 0);
  {
    tmp::function<int(int, int, int)> f1 = A(3);
    EXPECT_TRUE(A::count == 1);
    DisableAllocationGuard guard;
    ((void)guard);
    f1.swap(f1);
    EXPECT_TRUE(A::count == 1);
    RTTI_ASSERT(f1.target<A>()->id() == 3);
  }
  EXPECT_TRUE(globalMemCounter.checkOutstandingNewEq(0));
  EXPECT_TRUE(A::count == 0);
}
