#include "../../benchmark/report.hpp"
#include <chrono>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <sstream>

using namespace std::chrono_literals;
using testing::HasSubstr;

TEST(BenchmarkReport, PrintsStableHeaderAndProgramFields) {
  std::ostringstream output;
  Options options{.trials = 2, .threads = {4, 1}};
  Program program{.group = "smoke", .name = "arithmetic"};
  ProgramSummary summary{.totalCompileTime = 10ns,
                         .maxCompileTime = 6ns,
                         .totalRunTime = 20ns,
                         .maxRunTime = 12ns,
                         .totalTime = 30ns,
                         .maxTotalTime = 18ns,
                         .executedInstructions = 50};

  printHeader(output, "benchmarks", options);
  printProgramSummary(output, program, 4, 2, summary);

  EXPECT_THAT(output.str(), HasSubstr("Benchmark root: benchmarks\n"));
  EXPECT_THAT(output.str(), HasSubstr("Trials: 2\nThreads: 4 1\n"));
  EXPECT_THAT(output.str(), HasSubstr("arithmetic"));
  EXPECT_THAT(output.str(), HasSubstr("threads=4"));
  EXPECT_THAT(output.str(), HasSubstr("instructions=50"));
}

TEST(BenchmarkReport, PrintsAggregateAndRejectsZeroInstructions) {
  std::ostringstream output;
  printAggregateSummary(output,
                        {.totalRunTime = 20ns,
                         .executedInstructions = 4,
                         .byThread = {{.threads = 1,
                                       .totalRunTime = 6ns,
                                       .estimatedStartupTime = 1ns,
                                       .trialCount = 2,
                                       .executedInstructions = 3},
                                      {.threads = 4,
                                       .totalRunTime = 14ns,
                                       .estimatedStartupTime = 3ns,
                                       .trialCount = 2,
                                       .executedInstructions = 1}}});
  EXPECT_THAT(output.str(),
              HasSubstr("threads=1 time_per_bytecode_instruction=1.333ns"));
  EXPECT_THAT(output.str(),
              HasSubstr("threads=4 time_per_bytecode_instruction=8.000ns"));
  EXPECT_THAT(
      output.str(),
      HasSubstr("overall time_per_bytecode_instruction=3.000ns"));
  EXPECT_THAT(output.str(), HasSubstr("estimated_startup_time=1ns"));
  EXPECT_THAT(output.str(), HasSubstr("adjusted_run_time=4ns"));
  EXPECT_THAT(output.str(), HasSubstr("total_executed_instructions=4"));

  EXPECT_THROW(printAggregateSummary(output, {}), std::runtime_error);
}

TEST(BenchmarkReport, ClampsRuntimeAfterStartupAdjustmentToZero) {
  std::ostringstream output;
  printAggregateSummary(output,
                        {.totalRunTime = 2ns,
                         .executedInstructions = 1,
                         .byThread = {{.threads = 4,
                                       .totalRunTime = 2ns,
                                       .estimatedStartupTime = 3ns,
                                       .trialCount = 1,
                                       .executedInstructions = 1}}});

  EXPECT_THAT(output.str(),
              HasSubstr("threads=4 time_per_bytecode_instruction=0.000ns"));
  EXPECT_THAT(output.str(), HasSubstr("adjusted_run_time=0ns"));
  EXPECT_THAT(
      output.str(),
      HasSubstr("overall time_per_bytecode_instruction=0.000ns"));
}
