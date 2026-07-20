#pragma once

#include "../cliUtils.hpp"
#include "../exitCode.hpp"
#include <istream>

struct ExecutionStats;

// Executes an in-memory bytecode stream and optionally records executor stats.
void executeBytecode(const CliArgs &args, std::istream &bytecode,
                     ExecutionStats *stats = nullptr);

class Interpreter {
  const CliArgs &args;

public:
  Interpreter(const CliArgs &args);
  ExitCode interpret(std::istream &stream, ExecutionStats *stats = nullptr);
};
