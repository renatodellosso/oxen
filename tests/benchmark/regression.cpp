#include "../../benchmark/application.hpp"
#include "../../benchmark/discovery.hpp"
#include "../../benchmark/runner.hpp"
#include <algorithm>
#include <cstdint>
#include <gtest/gtest.h>
#include <sstream>
#include <string>
#include <vector>

namespace {

const std::filesystem::path bundledBenchmarkRoot = "benchmark/benchmarks";

} // namespace

TEST(BenchmarkRegression, EveryBundledProgramCanBeMeasured) {
  for (const Program &program : discoverPrograms(bundledBenchmarkRoot)) {
    SCOPED_TRACE(program.path.string());
    EXPECT_NO_THROW(runTrial(program, 1));
  }
}

TEST(BenchmarkRegression, BundledSuiteCompletesSuccessfully) {
  char executable[] = "Benchmark";
  char trialsOption[] = "--trials";
  char trialsValue[] = "1";
  char threadsOption[] = "--threads";
  char threadsValue[] = "1";
  char *arguments[] = {executable, trialsOption, trialsValue, threadsOption,
                       threadsValue};
  std::ostringstream output;
  std::ostringstream error;

  EXPECT_EQ(runApplication(5, arguments, output, error), 0) << error.str();
}
