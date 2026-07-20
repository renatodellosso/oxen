#include "interpreter.hpp"
#include "../color.hpp"
#include "../logging.hpp"
#include "../utils.hpp"
#include "bytecodeParser.hpp"
#include "executor.hpp"
#include "subprogram.hpp"
#include <chrono>
#include <format>
#include <memory>
#include <stdexcept>
#include <utility>
#include <vector>

#define LOCATION "Interpreter"

ExitCode executeBytecode(const CliArgs &args, std::istream &bytecode,
                         ExecutionStats *stats) {
  std::vector<Instruction> instructions;
  try {
    BytecodeParser parser(args, instructions, bytecode);
    parser.buildInstructions();
  } catch (const std::runtime_error &err) {
    logError(LOCATION, "{}",
             colorize(std::format("Encountered error parsing bytecode: {}",
                                  err.what()),
                      Color::Red));
    return ExitCode::BytecodeParseError;
  }

  try {
    auto instructionStore =
        std::make_shared<std::vector<Instruction>>(std::move(instructions));
    auto program = std::make_shared<Subprogram>(instructionStore);
    program->setSubprogramPointers(program);

    Executor executor(args, *program, stats);
    executor.startExecution();
  } catch (const std::runtime_error &err) {
    logError(LOCATION, "{}",
             colorize(std::format("Encountered error executing bytecode: {}",
                                  err.what()),
                      Color::Red));
    return ExitCode::ExecutionError;
  }

  return ExitCode::Ok;
}

Interpreter::Interpreter(const CliArgs &args) : args(args) {}

ExitCode Interpreter::interpret(std::istream &stream, ExecutionStats *stats) {
  auto start = std::chrono::steady_clock::now();

  if (args.verbose)
    log(LOCATION, "Interpreting file '{}'...", args.target);

  ExitCode exitCode = executeBytecode(args, stream, stats);
  if (exitCode != ExitCode::Ok)
    return exitCode;

  auto end = std::chrono::steady_clock::now();

  if (args.verbose)
    log(LOCATION, "Finished in {}", formatNs(end - start));

  return ExitCode::Ok;
}
