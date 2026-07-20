#pragma once

#include "../cliUtils.hpp"
#include "../exitCode.hpp"
#include <istream>

struct ExecutionStats;

class Interpreter {
  const CliArgs &args;

public:
  Interpreter(const CliArgs &args);
  ExitCode interpret(std::istream &stream, ExecutionStats *stats = nullptr);
};
