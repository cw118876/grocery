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

// Make sure we can use std::function with a type that has a hostile overload
// of operator&().

#include "function/function.hpp"
#include "gtest/gtest.h"
#include "operator_hijacker.h"

struct TrapAddressof : operator_hijacker {
    int operator()() const { return 1; }
};

TEST(FunctionFunc, AddressOf) {
    tmp::function<int()> f = TrapAddressof();
    EXPECT_TRUE(f() == 1);
}
