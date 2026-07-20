#include "runner.hpp"
#include "../src/cliUtils.hpp"
#include "../src/interpreter/executor.hpp"
#include "../src/programExecution.hpp"
#include "../src/streamRedirect.hpp"
#include <chrono>
#include <format>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>

namespace benchmarking {
namespace {

using Clock = std::chrono::steady_clock;

class CoutSilencer {
  std::ostringstream sink;
  ScopedStreamRedirect redirect;

public:
  CoutSilencer() : redirect(std::cout, sink.rdbuf()) {}

  CoutSilencer(const CoutSilencer &) = delete;
  CoutSilencer &operator=(const CoutSilencer &) = delete;
};

} // namespace

TrialResult runTrial(const Program &program, int threads) {
  CliArgs args = {.target = program.path.string(),
                  .outputFile = std::nullopt,
                  .mode = CliMode::CompileAndInterpret,
                  .verbose = false,
                  .threads = threads,
                  .debugBytecode = false};

  std::ifstream source(program.path);
  if (!source.is_open())
    throw std::runtime_error(std::format(
        "Could not open benchmark source '{}'", program.path.string()));

  auto compileStart = Clock::now();
  std::string bytecode;
  ExitCode compileExit = compileToBytecode(args, source, bytecode);
  auto compileEnd = Clock::now();
  if (compileExit != ExitCode::Ok)
    throw std::runtime_error(
        std::format("Compilation failed for '{}' with exit code {}",
                    program.path.string(), static_cast<int>(compileExit)));

  std::istringstream bytecodeStream(bytecode);
  ExecutionStats stats;
  auto runStart = Clock::now();
  ExitCode executionExit;
  {
    CoutSilencer silencer;
    executionExit = executeBytecode(args, bytecodeStream, &stats);
  }
  auto runEnd = Clock::now();
  if (executionExit != ExitCode::Ok)
    throw std::runtime_error(
        std::format("Execution failed for '{}' with exit code {}",
                    program.path.string(), static_cast<int>(executionExit)));

  return {.compileTime =
              std::chrono::duration_cast<Nanoseconds>(compileEnd - compileStart),
          .runTime =
              std::chrono::duration_cast<Nanoseconds>(runEnd - runStart),
          .executedInstructions =
              stats.executedInstructions.load(std::memory_order_relaxed)};
}

} // namespace benchmarking
