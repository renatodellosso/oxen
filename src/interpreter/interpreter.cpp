#include "interpreter.hpp"
#include "../color.hpp"
#include "../error.hpp"
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

void executeBytecode(const CliArgs &args, std::istream &bytecode,
                     ExecutionStats *stats) {
  std::vector<Instruction> instructions;
  try {
    BytecodeParser parser(args, instructions, bytecode);
    parser.buildInstructions();
  } catch (const std::runtime_error &err) {
    throw Error(
        ExitCode::BytecodeParseError,
        std::format("Encountered error parsing bytecode: {}", err.what()));
  }

  try {
    // Subprogram owns the parsed instruction buffer from this point onward.
    auto instructionStore =
        std::make_shared<std::vector<Instruction>>(std::move(instructions));
    auto program = std::make_shared<Subprogram>(instructionStore);
    program->setSubprogramPointers(program);

    Executor executor(args, *program, stats);
    executor.startExecution();
  } catch (const std::runtime_error &err) {
    throw Error(
        ExitCode::ExecutionError,
        std::format("Encountered error executing bytecode: {}", err.what()));
  }
}

Interpreter::Interpreter(const CliArgs &args) : args(args) {}

ExitCode Interpreter::interpret(std::istream &stream, ExecutionStats *stats) {
  auto start = std::chrono::steady_clock::now();

  if (args.verbose)
    log(LOCATION, "Interpreting file '{}'...", args.target);

  try {
    executeBytecode(args, stream, stats);
  } catch (const Error &error) {
    logError(LOCATION, "{}", colorize(error.what(), Color::Red));
    return error.getCode();
  }

  auto end = std::chrono::steady_clock::now();

  if (args.verbose)
    log(LOCATION, "Finished in {}", formatNs(end - start));

  return ExitCode::Ok;
}
