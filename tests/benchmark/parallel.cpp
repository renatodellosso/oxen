#include "../../benchmark/discovery.hpp"
#include "../../benchmark/runner.hpp"
#include <algorithm>
#include <cstdint>
#include <gtest/gtest.h>
#include <string>
#include <utility>
#include <vector>

namespace {

const std::filesystem::path bundledBenchmarkRoot = "benchmark/benchmarks";

} // namespace

TEST(BenchmarkParallel, CoarseProgramsAreStableAcrossWorkerCounts) {
  const std::vector<Program> programs = discoverPrograms(bundledBenchmarkRoot);
  const std::vector<std::pair<std::string, std::uint64_t>> coarsePrograms = {
      {"balancedStringQuadruplingFanOut", 1500},
      {"balancedStringQuintuplingFanOut", 1500},
      {"bulkStringFanOut", 100},
      {"stringDoublingFanOut", 2000},
      {"stringGrowthFanOut", 25000}};

  for (const auto &[name, minimumInstructions] : coarsePrograms) {
    auto program = std::find_if(
        programs.begin(), programs.end(), [&](const Program &candidate) {
          return candidate.group == "parallel" && candidate.name == name;
        });
    ASSERT_NE(program, programs.end()) << name;

    TrialResult baseline = runTrial(*program, 1);
    EXPECT_GE(baseline.executedInstructions, minimumInstructions) << name;

    for (int threads : {2, 4, 8, 16}) {
      SCOPED_TRACE(name + " with " + std::to_string(threads) + " workers");
      TrialResult concurrent = runTrial(*program, threads);
      EXPECT_EQ(concurrent.executedInstructions,
                baseline.executedInstructions);
    }
  }
}
