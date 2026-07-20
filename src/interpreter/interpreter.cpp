#include "interpreter.hpp"
#include "../logging.hpp"
#include "../programExecution.hpp"
#include "../utils.hpp"
#include <chrono>

#define LOCATION "Interpreter"

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
