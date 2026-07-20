#include "report.hpp"
#include "../src/utils.hpp"
#include <format>
#include <stdexcept>

namespace benchmarking {
namespace {

std::string formatAverage(Nanoseconds total, int count) {
  return formatNs(total / count);
}

} // namespace

void printHeader(std::ostream &output, const std::filesystem::path &root,
                 const Options &options) {
  output << std::format("Benchmark root: {}\n", root.string());
  output << std::format("Trials: {}\n", options.trials);
  output << "Threads:";
  for (int threads : options.threads)
    output << " " << threads;
  output << "\n\n";
}

void printProgramSummary(std::ostream &output, const Program &program,
                         int threads, int trials,
                         const ProgramSummary &summary) {
  output << std::format(
      "  {:<24} threads={:<3} trials={:<4} "
      "compile_avg={:<12} compile_max={:<12} "
      "run_avg={:<12} run_max={:<12} "
      "total_avg={:<12} total_max={:<12} instructions={}\n",
      program.name, threads, trials,
      formatAverage(summary.totalCompileTime, trials),
      formatNs(summary.maxCompileTime),
      formatAverage(summary.totalRunTime, trials),
      formatNs(summary.maxRunTime), formatAverage(summary.totalTime, trials),
      formatNs(summary.maxTotalTime), summary.executedInstructions);
}

void printAggregateSummary(std::ostream &output,
                           const AggregateSummary &summary) {
  if (summary.executedInstructions == 0)
    throw std::runtime_error(
        "No bytecode instructions were executed by the benchmarks");

  double nsPerInstruction =
      static_cast<double>(summary.totalRunTime.count()) /
      static_cast<double>(summary.executedInstructions);
  output << std::format(
      "\ntime_per_bytecode_instruction={:.3f}ns "
      "total_run_time={} total_executed_instructions={}\n",
      nsPerInstruction, formatNs(summary.totalRunTime),
      summary.executedInstructions);
}

} // namespace benchmarking
