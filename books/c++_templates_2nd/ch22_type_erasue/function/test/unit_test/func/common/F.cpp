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

// function(F);

#include "function//function.hpp"
#include "gtest/gtest.h"

#include "test_macros.h"
#include "count_new.h"

 namespace {
class A
{
    int data_[10];
public:
    static int count;

    A()
    {
        ++count;
        for (int i = 0; i < 10; ++i)
            data_[i] = i;
    }

    A(const A&) {++count;}

    ~A() {--count;}

    int operator()(int i) const
    {
        for (int j = 0; j < 10; ++j)
            i += data_[j];
        return i;
    }

    int foo(int) const {return 1;}
};

int A::count = 0;

int g(int) {return 0;}

#if TEST_STD_VER >= 11
struct RValueCallable {
    template <class ...Args>
    void operator()(Args&&...) && {}
};
struct LValueCallable {
    template <class ...Args>
    void operator()(Args&&...) & {}
};
#endif
 }

TEST(FunctionFuncCommon, F)
{
    globalMemCounter.reset();
    EXPECT_TRUE(globalMemCounter.checkOutstandingNewEq(0));
    {
    tmp::function<int(int)> f = A();
    EXPECT_TRUE(A::count == 1);
    EXPECT_TRUE(globalMemCounter.checkOutstandingNewLessThanOrEqual(1));
    RTTI_ASSERT(f.target<A>());
    RTTI_ASSERT(f.target<int(*)(int)>() == 0);
    }
    EXPECT_TRUE(A::count == 0);
    EXPECT_TRUE(globalMemCounter.checkOutstandingNewEq(0));
    {
    tmp::function<int(int)> f = g;
    EXPECT_TRUE(globalMemCounter.checkOutstandingNewEq(0));
    RTTI_ASSERT(f.target<int(*)(int)>());
    RTTI_ASSERT(f.target<A>() == 0);
    }
    EXPECT_TRUE(globalMemCounter.checkOutstandingNewEq(0));
    {
    tmp::function<int(int)> f = (int (*)(int))0;
    EXPECT_TRUE(!f);
    EXPECT_TRUE(globalMemCounter.checkOutstandingNewEq(0));
    RTTI_ASSERT(f.target<int(*)(int)>() == 0);
    RTTI_ASSERT(f.target<A>() == 0);
    }
    {
    tmp::function<int(const A*, int)> f = &A::foo;
    EXPECT_TRUE(f);
    EXPECT_TRUE(globalMemCounter.checkOutstandingNewEq(0));
    RTTI_ASSERT(f.target<int (A::*)(int) const>() != 0);
    }
    {
      tmp::function<void(int)> f(&g);
      EXPECT_TRUE(f);
      RTTI_ASSERT(f.target<int(*)(int)>() != 0);
      f(1);
    }
    {
        tmp::function <void()> f(static_cast<void (*)()>(0));
        EXPECT_TRUE(!f);
    }
#if TEST_STD_VER >= 11
    {
        using Fn = tmp::function<void(int, int, int)>;
        static_assert(std::is_constructible<Fn, LValueCallable&>::value, "");
        static_assert(std::is_constructible<Fn, LValueCallable>::value, "");
        static_assert(!std::is_constructible<Fn, RValueCallable&>::value, "");
        static_assert(!std::is_constructible<Fn, RValueCallable>::value, "");
    }
#endif
}