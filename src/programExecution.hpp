#pragma once

#include "cliUtils.hpp"
#include "exitCode.hpp"
#include <istream>
#include <string>

struct ExecutionStats;

ExitCode compileToBytecode(const CliArgs &args, std::istream &source,
                           std::string &bytecode);

ExitCode executeBytecode(const CliArgs &args, std::istream &bytecode,
                         ExecutionStats *stats = nullptr);
