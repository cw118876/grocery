#include "function/function.hpp"
#include "gtest/gtest.h"

#include <exception>
#include <type_traits>

TEST(BadFunctionCall, except) {
  auto is_base = std::is_base_of<std::exception, tmp::bad_function_call>::value;
  EXPECT_EQ(is_base, true);
}
