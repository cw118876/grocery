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

// explicit operator bool() const

#include <type_traits>

#include "test_macros.h"

#include "function/function.hpp"
#include "gtest/gtest.h"

static int g(int) {
  return 0;
}

TEST(FunctionFunc, operatorBool) {
  auto bool_to_fun = std::is_constructible<bool, tmp::function<void()> >::value;
  EXPECT_EQ(bool_to_fun, true);
  auto fun_to_bool = !std::is_convertible<tmp::function<void()>, bool>::value;
  EXPECT_EQ(fun_to_bool, true);

  {
    tmp::function<int(int)> f;
    EXPECT_EQ(bool(!f), true);
    f = g;
    EXPECT_EQ(bool(f), true);
  }
}
