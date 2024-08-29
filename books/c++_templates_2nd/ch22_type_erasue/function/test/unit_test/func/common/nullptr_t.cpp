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

// function(nullptr_t);
#include "function/function.hpp"
#include "gtest/gtest.h"

#include "test_macros.h"

static int g(int) {return 0;}

TEST(FunctionFuncCommon, Nullptr)
{
    tmp::function<int(int)> f(nullptr);
    EXPECT_TRUE(!f);

}

TEST(FunctionFuncCommon, NullptrCompare)
{
    {
    tmp::function<int(int)> f;
    EXPECT_TRUE(f == nullptr);
    EXPECT_TRUE(nullptr == f);
    f = g;
    EXPECT_TRUE(f != nullptr);
    EXPECT_TRUE(nullptr != f);
    }
}