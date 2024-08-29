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

// function();

#include "function/function.hpp"

#include "gtest/gtest.h"
#include "test_macros.h"

TEST(FunctionFuncCommon, Default) {
  tmp::function<int(int)> f;
  EXPECT_TRUE(!f);
}
