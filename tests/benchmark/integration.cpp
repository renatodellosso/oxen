#include "../../benchmark/application.hpp"
#include "../../benchmark/runner.hpp"
#include "../../src/cliUtils.hpp"
#include "../../src/compiler/compiler.hpp"
#include "../../src/error.hpp"
#include "../../src/interpreter/executor.hpp"
#include "../../src/interpreter/interpreter.hpp"
#include "../../src/streamRedirect.hpp"
#include <chrono>
#include <filesystem>
#include <fstream>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <sstream>
#include <stdexcept>
#include <string>

using testing::HasSubstr;

namespace {

class BenchmarkFixture : public testing::Test {
protected:
  std::filesystem::path root;

  void SetUp() override {
    auto suffix = std::chrono::steady_clock::now().time_since_epoch().count();
    root = std::filesystem::temp_directory_path() /
           ("parallel-benchmark-integration-" + std::to_string(suffix));
    std::filesystem::create_directories(root / "smoke");
  }

  void TearDown() override { std::filesystem::remove_all(root); }

  std::filesystem::path writeProgram(const std::string &name,
                                     const std::string &source) {
    auto path = root / "smoke" / name;
    std::ofstream output(path);
    output << source;
    return path;
  }
};

} // namespace

TEST_F(BenchmarkFixture, SharedExecutionCompilesAndCollectsStatistics) {
  CliArgs args{.target = "memory",
               .outputFile = std::nullopt,
               .mode = CliMode::CompileAndInterpret,
               .verbose = false,
               .threads = 1,
               .debugBytecode = false};
  std::istringstream source("int value = 1; value = value + 2;");
  std::string bytecode;
  ASSERT_EQ(compileToBytecode(args, source, bytecode), ExitCode::Ok);

  std::istringstream bytecodeStream(bytecode);
  ExecutionStats stats;
  EXPECT_NO_THROW(executeBytecode(args, bytecodeStream, &stats));
  EXPECT_GT(stats.executedInstructions.load(), 0);
}

TEST_F(BenchmarkFixture, RunsCompleteBenchmarkWorkflow) {
  writeProgram("simple.p", "int value = 1; value = value + 2;");
  std::ostringstream output;

  auto aggregate = run({.trials = 1, .threads = {1}}, root, output);

  EXPECT_GT(aggregate.executedInstructions, 0);
  EXPECT_GE(aggregate.totalRunTime.count(), 0);
  EXPECT_THAT(output.str(), HasSubstr("smoke\n"));
  EXPECT_THAT(output.str(), HasSubstr("simple"));
  EXPECT_THAT(output.str(), HasSubstr("threads=1"));
  EXPECT_THAT(output.str(), HasSubstr("time_per_bytecode_instruction="));
}

TEST_F(BenchmarkFixture, ReportsInvalidSourceWithProgramPath) {
  auto path = writeProgram("invalid.p", "this is not valid;");
  Program program{.group = "smoke", .name = "invalid", .path = path};
  std::ostringstream ignored;
  ScopedStreamRedirect redirect(std::cout, ignored.rdbuf());

  try {
    runTrial(program, 1);
    FAIL() << "Expected compilation to fail";
  } catch (const std::runtime_error &error) {
    EXPECT_THAT(error.what(), HasSubstr(path.string()));
    EXPECT_THAT(error.what(), HasSubstr("Compilation failed"));
  }
}

TEST_F(BenchmarkFixture, ReportsUnreadableSourceWithProgramPath) {
  auto path = root / "smoke" / "missing.p";
  Program program{.group = "smoke", .name = "missing", .path = path};
  EXPECT_THROW(
      {
        try {
          runTrial(program, 1);
        } catch (const std::runtime_error &error) {
          EXPECT_THAT(error.what(), HasSubstr(path.string()));
          throw;
        }
      },
      std::runtime_error);
}

TEST_F(BenchmarkFixture, ReportsExecutionCauseWithProgramPath) {
  auto path = writeProgram("invalid-operation.p", "bool value = true < false;");
  Program program{.group = "smoke", .name = "invalid-operation", .path = path};

  try {
    runTrial(program, 1);
    FAIL() << "Expected execution to fail";
  } catch (const Error &error) {
    EXPECT_EQ(error.getCode(), ExitCode::ExecutionError);
    EXPECT_THAT(error.what(), HasSubstr(path.string()));
    EXPECT_THAT(error.what(), HasSubstr("Invalid arg types"));
  }
}

TEST(StreamRedirect, RestoresStreamAfterException) {
  std::ostringstream destination;
  std::ostringstream redirected;
  try {
    ScopedStreamRedirect redirect(destination, redirected.rdbuf());
    destination << "redirected";
    throw std::runtime_error("stop");
  } catch (const std::runtime_error &) {
  }

  destination << "restored";
  EXPECT_EQ(redirected.str(), "redirected");
  EXPECT_EQ(destination.str(), "restored");
}
