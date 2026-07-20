#include "programExecution.hpp"
#include "color.hpp"
#include "compiler/compiler.hpp"
#include "interpreter/bytecodeParser.hpp"
#include "interpreter/executor.hpp"
#include "interpreter/subprogram.hpp"
#include "logging.hpp"
#include <format>
#include <memory>
#include <optional>
#include <stdexcept>
#include <utility>
#include <vector>

namespace {

constexpr const char *location = "ProgramExecution";

} // namespace

ExitCode compileToBytecode(const CliArgs &args, std::istream &source,
                           std::string &bytecode) {
  return compile(args, source, [&bytecode](std::string text) {
    bytecode = std::move(text);
    return std::nullopt;
  });
}

ExitCode executeBytecode(const CliArgs &args, std::istream &bytecode,
                         ExecutionStats *stats) {
  std::vector<Instruction> instructions;
  try {
    BytecodeParser parser(args, instructions, bytecode);
    parser.buildInstructions();
  } catch (const std::runtime_error &err) {
    logError(location, "{}",
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
    logError(location, "{}",
             colorize(std::format("Encountered error executing bytecode: {}",
                                  err.what()),
                      Color::Red));
    return ExitCode::ExecutionError;
  }

  return ExitCode::Ok;
}
