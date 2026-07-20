#pragma once

#include "types.hpp"
#include <filesystem>
#include <ostream>

AggregateSummary run(const Options &options,
                     const std::filesystem::path &benchmarkRoot,
                     std::ostream &output);
int runApplication(int argc, char *argv[], std::ostream &output,
                   std::ostream &error);
