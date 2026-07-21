#pragma once

#include <chrono>
#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

using Nanoseconds = std::chrono::nanoseconds;

struct Program {
  std::string group;
  std::string name;
  std::filesystem::path path;
};

struct Options {
  int trials = 10;
  std::vector<int> threads = {1, 2, 4, 8, 16};
};

struct TrialResult {
  Nanoseconds compileTime = Nanoseconds::zero();
  Nanoseconds runTime = Nanoseconds::zero();
  std::uint64_t executedInstructions = 0;
};

struct ProgramSummary {
  Nanoseconds totalCompileTime = Nanoseconds::zero();
  Nanoseconds maxCompileTime = Nanoseconds::zero();
  Nanoseconds totalRunTime = Nanoseconds::zero();
  Nanoseconds maxRunTime = Nanoseconds::zero();
  Nanoseconds totalTime = Nanoseconds::zero();
  Nanoseconds maxTotalTime = Nanoseconds::zero();
  std::uint64_t executedInstructions = 0;
};

struct ThreadAggregateSummary {
  int threads = 0;
  Nanoseconds totalRunTime = Nanoseconds::zero();
  std::uint64_t executedInstructions = 0;
};

struct AggregateSummary {
  Nanoseconds totalRunTime = Nanoseconds::zero();
  std::uint64_t executedInstructions = 0;
  std::vector<ThreadAggregateSummary> byThread;
};
