#include "application.hpp"
#include "discovery.hpp"
#include "options.hpp"
#include "report.hpp"
#include "results.hpp"
#include "runner.hpp"
#include <iostream>
#include <stdexcept>
#include <string_view>
#include <vector>

namespace benchmarking {

AggregateSummary run(const Options &options,
                     const std::filesystem::path &benchmarkRoot,
                     std::ostream &output) {
  const std::vector<Program> programs = discoverPrograms(benchmarkRoot);
  AggregateSummary aggregate;
  std::string currentGroup;

  printHeader(output, benchmarkRoot, options);
  for (const Program &program : programs) {
    if (program.group != currentGroup) {
      currentGroup = program.group;
      output << currentGroup << '\n';
    }

    for (int threads : options.threads) {
      std::vector<TrialResult> trials;
      trials.reserve(options.trials);
      for (int trial = 0; trial < options.trials; ++trial)
        trials.push_back(runTrial(program, threads));

      ProgramSummary summary = summarize(trials);
      aggregate.executedInstructions += summary.executedInstructions;
      aggregate.totalRunTime += summary.totalRunTime;
      printProgramSummary(output, program, threads, options.trials, summary);
    }
  }

  printAggregateSummary(output, aggregate);
  return aggregate;
}

int runApplication(int argc, char *argv[], std::ostream &output,
                   std::ostream &error) {
  try {
    std::vector<std::string_view> arguments;
    arguments.reserve(argc > 0 ? argc - 1 : 0);
    for (int i = 1; i < argc; ++i)
      arguments.emplace_back(argv[i]);

    run(parseOptions(arguments), "benchmark/benchmarks", output);
    return 0;
  } catch (const std::exception &err) {
    error << "benchmark: " << err.what() << '\n';
    return 1;
  }
}

} // namespace benchmarking
