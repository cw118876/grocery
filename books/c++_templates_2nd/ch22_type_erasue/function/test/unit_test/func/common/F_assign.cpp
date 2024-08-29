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

// template<class F>
//   requires CopyConstructible<F> && Callable<F, ArgTypes..>
//         && Convertible<Callable<F, ArgTypes...>::result_type
//   operator=(F f);

#include "function/function.hpp"

#include "test_macros.h"
#include "count_new.h"
#include "gtest/gtest.h"

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

#define static_EXPECT_TRUE(val, val1) EXPECT_TRUE(val)

TEST(FunctionFuncCommon, FAssign)
{
    globalMemCounter.reset();
    EXPECT_TRUE(globalMemCounter.checkOutstandingNewEq(0));
    {
    tmp::function<int(int)> f;
    f = A();
    EXPECT_TRUE(A::count == 1);
    EXPECT_TRUE(globalMemCounter.checkOutstandingNewEq(1));
    RTTI_ASSERT(f.target<A>());
    RTTI_ASSERT(f.target<int(*)(int)>() == 0);
    }
    EXPECT_TRUE(A::count == 0);
    EXPECT_TRUE(globalMemCounter.checkOutstandingNewEq(0));
    {
    tmp::function<int(int)> f;
    f = g;
    EXPECT_TRUE(globalMemCounter.checkOutstandingNewEq(0));
    RTTI_ASSERT(f.target<int(*)(int)>());
    RTTI_ASSERT(f.target<A>() == 0);
    }
    EXPECT_TRUE(globalMemCounter.checkOutstandingNewEq(0));
    {
    tmp::function<int(int)> f;
    f = (int (*)(int))0;
    EXPECT_TRUE(!f);
    EXPECT_TRUE(globalMemCounter.checkOutstandingNewEq(0));
    RTTI_ASSERT(f.target<int(*)(int)>() == 0);
    RTTI_ASSERT(f.target<A>() == 0);
    }
    {
    tmp::function<int(const A*, int)> f;
    f = &A::foo;
    EXPECT_TRUE(f);
    EXPECT_TRUE(globalMemCounter.checkOutstandingNewEq(0));
    RTTI_ASSERT(f.target<int (A::*)(int) const>() != 0);
    }
    {
    tmp::function<void(int)> f;
    f = &g;
    EXPECT_TRUE(f);
    // RTTI_ASSERT(f.target<int(*)(int)>() != 0);
    f(1);
    }
#if TEST_STD_VER >= 11
    {
        using Fn = tmp::function<void(int, int, int)>;
        static_EXPECT_TRUE((std::is_assignable<Fn&, LValueCallable&>::value), "");
        static_EXPECT_TRUE((std::is_assignable<Fn&, LValueCallable>::value), "");
        static_EXPECT_TRUE((!std::is_assignable<Fn&, RValueCallable&>::value), "");
        static_EXPECT_TRUE((!std::is_assignable<Fn&, RValueCallable>::value), "");
    }
    {
        using Fn = tmp::function<void(int, int, int)>;
        static_EXPECT_TRUE((std::is_assignable<Fn&, Fn&&>::value), "");
    }
    {
        using F1 = tmp::function<void(int, int)>;
        using F2 = tmp::function<void(int, int, int)>;
        static_EXPECT_TRUE((!std::is_assignable<F1&, F2&&>::value), "");
    }
    {
        using F1 = tmp::function<int(int, int)>;
        using F2 = tmp::function<A  (int, int)>;
        static_EXPECT_TRUE((!std::is_assignable<F1&, F2&&>::value), "");
        static_EXPECT_TRUE((!std::is_assignable<F2&, F1&&>::value), "");
    }
#endif
}
