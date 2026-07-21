#include "report.hpp"
#include "../src/utils.hpp"
#include <format>
#include <stdexcept>

namespace {

std::string formatAverage(Nanoseconds total, int count) {
  return formatNs(total / count);
}

double nanosecondsPerInstruction(Nanoseconds totalRunTime,
                                 std::uint64_t executedInstructions) {
  return static_cast<double>(totalRunTime.count()) /
         static_cast<double>(executedInstructions);
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

  output << '\n';
  for (const ThreadAggregateSummary &threadSummary : summary.byThread) {
    if (threadSummary.executedInstructions == 0)
      throw std::runtime_error(std::format(
          "No bytecode instructions were executed with {} threads",
          threadSummary.threads));

    output << std::format(
        "threads={} time_per_bytecode_instruction={:.3f}ns "
        "total_run_time={} total_executed_instructions={}\n",
        threadSummary.threads,
        nanosecondsPerInstruction(threadSummary.totalRunTime,
                                  threadSummary.executedInstructions),
        formatNs(threadSummary.totalRunTime),
        threadSummary.executedInstructions);
  }

  output << std::format(
      "overall time_per_bytecode_instruction={:.3f}ns "
      "total_run_time={} total_executed_instructions={}\n",
      nanosecondsPerInstruction(summary.totalRunTime,
                                summary.executedInstructions),
      formatNs(summary.totalRunTime),
      summary.executedInstructions);
}
