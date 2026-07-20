#pragma once

#include "../cliUtils.hpp"
#include "../exitCode.hpp"
#include <functional>
#include <istream>
#include <optional>
#include <string>

ExitCode
compile(const CliArgs &args, std::istream &inputStream,
        std::function<std::optional<std::string>(std::string)> writeOutput);

// Compiles directly to memory for callers that do not need an output file.
ExitCode compileToBytecode(const CliArgs &args, std::istream &source,
                           std::string &bytecode);
