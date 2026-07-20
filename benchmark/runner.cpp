#include "runner.hpp"
#include "../src/cliUtils.hpp"
#include "../src/compiler/compiler.hpp"
#include "../src/interpreter/executor.hpp"
#include "../src/interpreter/interpreter.hpp"
#include "../src/streamRedirect.hpp"
#include <chrono>
#include <format>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>

namespace {

using Clock = std::chrono::steady_clock;

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
    std::ostringstream ignoredOutput;
    ScopedStreamRedirect redirect(std::cout, ignoredOutput.rdbuf());
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
