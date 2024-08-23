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

// template <MoveConstructible  R, MoveConstructible ... ArgTypes>
//   void swap(function<R(ArgTypes...)>&, function<R(ArgTypes...)>&) noexcept;

#include <cassert>
#include <cstdlib>

#include "function/function.hpp"
#include "gtest/gtest.h"

#include "count_new.h"
#include "test_macros.h"

class A {
  int data_[10];

 public:
  static int count;

  explicit A(int j) {
    ++count;
    data_[0] = j;
  }

  A(const A& a) {
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

  int id() const { return data_[0]; }
};

int A::count = 0;

int g(int) {
  return 0;
}
int h(int) {
  return 1;
}

TEST(FunctionFunc, swap) {
  globalMemCounter.reset();
  EXPECT_EQ(globalMemCounter.checkOutstandingNewEq(0), true);
  {
    tmp::function<int(int)> s;
    tmp::function<int(int)> f = nullptr;
    tmp::function<int(int)> f1 = A(1);
    tmp::function<int(int)> f2 = A(2);
#if TEST_STD_VER >= 11
    static_assert(noexcept(swap(f1, f2)), "");
#endif
    EXPECT_EQ(A::count, 2);
    EXPECT_EQ(globalMemCounter.checkOutstandingNewLessThanOrEqual(2), true);
    swap(f1, f2);
    EXPECT_EQ(A::count, 2);
    EXPECT_EQ(globalMemCounter.checkOutstandingNewLessThanOrEqual(2), true);
  }
  EXPECT_EQ(A::count, 0);
  EXPECT_EQ(globalMemCounter.checkOutstandingNewEq(0), true);
  {
    tmp::function<int(int)> f1 = A(1);
    tmp::function<int(int)> f2 = g;
#if TEST_STD_VER >= 11
    static_assert(noexcept(swap(f1, f2)), "");
#endif
    EXPECT_EQ(A::count, 1);
    EXPECT_EQ(globalMemCounter.checkOutstandingNewLessThanOrEqual(1), true);
    swap(f1, f2);
    EXPECT_EQ(A::count, 1);
    EXPECT_EQ(globalMemCounter.checkOutstandingNewLessThanOrEqual(1), true);
  }
  EXPECT_EQ(A::count, 0);
  EXPECT_EQ(globalMemCounter.checkOutstandingNewEq(0), true);
  {
    tmp::function<int(int)> f1 = g;
    tmp::function<int(int)> f2 = A(1);
#if TEST_STD_VER >= 11
    static_assert(noexcept(swap(f1, f2)), "");
#endif
    EXPECT_EQ(A::count, 1);
    EXPECT_EQ(globalMemCounter.checkOutstandingNewLessThanOrEqual(1), true);
    swap(f1, f2);
    EXPECT_EQ(A::count, 1);
    EXPECT_EQ(globalMemCounter.checkOutstandingNewLessThanOrEqual(1), true);
  }
  EXPECT_EQ(A::count, 0);
  EXPECT_EQ(globalMemCounter.checkOutstandingNewEq(0), true);
  {
    tmp::function<int(int)> f1 = g;
    tmp::function<int(int)> f2 = h;
#if TEST_STD_VER >= 11
    static_assert(noexcept(swap(f1, f2)), "");
#endif
    EXPECT_EQ(A::count, 0);
    EXPECT_EQ(globalMemCounter.checkOutstandingNewLessThanOrEqual(0), true);
    swap(f1, f2);
    EXPECT_EQ(A::count, 0);
    EXPECT_EQ(globalMemCounter.checkOutstandingNewLessThanOrEqual(0), true);
  }
  EXPECT_EQ(A::count, 0);
  EXPECT_EQ(globalMemCounter.checkOutstandingNewEq(0), true);
}
