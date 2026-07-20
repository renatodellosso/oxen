#include "../../benchmark/results.hpp"
#include <chrono>
#include <gtest/gtest.h>

using namespace std::chrono_literals;

TEST(BenchmarkResults, AggregatesDurationsAndInstructions) {
  std::vector<benchmarking::TrialResult> trials = {
      {.compileTime = 2ns, .runTime = 5ns, .executedInstructions = 7},
      {.compileTime = 3ns, .runTime = 4ns, .executedInstructions = 11}};

  auto summary = benchmarking::summarize(trials);

  EXPECT_EQ(summary.totalCompileTime, 5ns);
  EXPECT_EQ(summary.maxCompileTime, 3ns);
  EXPECT_EQ(summary.totalRunTime, 9ns);
  EXPECT_EQ(summary.maxRunTime, 5ns);
  EXPECT_EQ(summary.totalTime, 14ns);
  EXPECT_EQ(summary.maxTotalTime, 7ns);
  EXPECT_EQ(summary.executedInstructions, 18);
}

TEST(BenchmarkResults, HandlesNoTrials) {
  auto summary = benchmarking::summarize({});
  EXPECT_EQ(summary.totalTime, 0ns);
  EXPECT_EQ(summary.executedInstructions, 0);
}
