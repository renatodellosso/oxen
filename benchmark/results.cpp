#include "results.hpp"
#include <algorithm>

namespace benchmarking {

ProgramSummary summarize(const std::vector<TrialResult> &trials) {
  ProgramSummary summary;
  for (const TrialResult &trial : trials) {
    summary.totalCompileTime += trial.compileTime;
    summary.maxCompileTime =
        std::max(summary.maxCompileTime, trial.compileTime);
    summary.totalRunTime += trial.runTime;
    summary.maxRunTime = std::max(summary.maxRunTime, trial.runTime);

    Nanoseconds totalTime = trial.compileTime + trial.runTime;
    summary.totalTime += totalTime;
    summary.maxTotalTime = std::max(summary.maxTotalTime, totalTime);
    summary.executedInstructions += trial.executedInstructions;
  }
  return summary;
}

} // namespace benchmarking
