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

// function& operator=(nullptr_t);

#include "function/function.hpp"
#include "gtest/gtest.h"

#include "count_new.h"

#include "test_macros.h"

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
};

int A::count = 0;

int g(int) {return 0;}
}

TEST(FunctionFuncCommon, nullptrAssign)
{
    globalMemCounter.reset();
    EXPECT_TRUE(globalMemCounter.checkOutstandingNewEq(0));
    {
    tmp::function<int(int)> f = A();
    EXPECT_TRUE(A::count == 1);
    EXPECT_TRUE(globalMemCounter.checkOutstandingNewLessThanOrEqual(1));
    RTTI_ASSERT(f.target<A>());
    f = nullptr;
    EXPECT_TRUE(A::count == 0);
    EXPECT_TRUE(globalMemCounter.checkOutstandingNewEq(0));
    RTTI_ASSERT(f.target<A>() == 0);
    }
    {
    tmp::function<int(int)> f = g;
    EXPECT_TRUE(globalMemCounter.checkOutstandingNewEq(0));
    RTTI_ASSERT(f.target<int(*)(int)>());
    RTTI_ASSERT(f.target<A>() == 0);
    f = nullptr;
    EXPECT_TRUE(globalMemCounter.checkOutstandingNewEq(0));
    RTTI_ASSERT(f.target<int(*)(int)>() == 0);
    }
}
