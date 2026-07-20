#include "../../benchmark/options.hpp"
#include <gtest/gtest.h>
#include <stdexcept>
#include <string_view>
#include <vector>

using benchmarking::parseOptions;

TEST(BenchmarkOptions, UsesDefaults) {
  auto options = parseOptions({});
  EXPECT_EQ(options.trials, 10);
  EXPECT_EQ(options.threads, (std::vector<int>{1, 2, 4, 8, 16}));
}

TEST(BenchmarkOptions, ParsesOverridesAndPreservesThreadOrder) {
  auto options =
      parseOptions({"--threads", "8,1,8", "--trials", "3"});
  EXPECT_EQ(options.trials, 3);
  EXPECT_EQ(options.threads, (std::vector<int>{8, 1, 8}));
}

TEST(BenchmarkOptions, RejectsUnknownAndMissingOptions) {
  EXPECT_THROW(parseOptions({"--unknown"}), std::runtime_error);
  EXPECT_THROW(parseOptions({"--trials"}), std::runtime_error);
  EXPECT_THROW(parseOptions({"--threads"}), std::runtime_error);
}

TEST(BenchmarkOptions, RejectsInvalidPositiveIntegers) {
  for (std::string_view value : {"0", "-1", "text", "1x",
                                 "999999999999999999999"}) {
    EXPECT_THROW(parseOptions({"--trials", value}), std::runtime_error)
        << value;
  }
}

TEST(BenchmarkOptions, RejectsMalformedThreadLists) {
  for (std::string_view value : {"", ",1", "1,", "1,,2", "1,0",
                                 "1,-2", "1,text"}) {
    EXPECT_THROW(parseOptions({"--threads", value}), std::runtime_error)
        << value;
  }
}
