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

ExitCode compileToBytecode(const CliArgs &args, std::istream &source,
                           std::string &bytecode);
