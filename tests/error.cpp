#include "../src/error.hpp"
#include <gtest/gtest.h>
#include <string>

TEST(Error, PreservesMessageAndExitCode) {
  Error error(ExitCode::ExecutionError, "execution failed");

  EXPECT_EQ(std::string(error.what()), "execution failed");
  EXPECT_EQ(error.getCode(), ExitCode::ExecutionError);
}
