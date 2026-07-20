#pragma once

#include "types.hpp"
#include <filesystem>
#include <ostream>

void printHeader(std::ostream &output, const std::filesystem::path &root,
                 const Options &options);
void printProgramSummary(std::ostream &output, const Program &program,
                         int threads, int trials,
                         const ProgramSummary &summary);
void printAggregateSummary(std::ostream &output,
                           const AggregateSummary &summary);
