# Benchmark architecture and interpretation

The benchmark executable measures the same compiler and interpreter entry
points used by normal compile-and-run operation. Its purpose is comparative
measurement and workload regression coverage; it does not assert wall-clock
speedups.

The detailed module overview is in
[`benchmark/ARCHITECTURE.md`](../../../benchmark/ARCHITECTURE.md). This document
focuses on what maintainers must preserve when implementation changes affect
measurement.

## Execution path

`runApplication()` parses `--trials` and `--threads` and calls `run()` in
[`benchmark/application.cpp`](../../../benchmark/application.cpp). `run()`:

1. discovers `.ox` files one group directory below `benchmark/benchmarks` via
   `discoverPrograms()`;
2. measures an empty-program startup estimate for every worker count;
3. calls `runTrial()` for every program, worker count, and trial;
4. aggregates results and prints per-program and per-thread summaries.

`runTrial()` in [`runner.cpp`](../../../benchmark/runner.cpp) calls
`compileToBytecode()` and `executeBytecode()` directly. `ScopedStreamRedirect`
suppresses program output so it cannot corrupt the report. Execution time
therefore includes bytecode parsing, program construction, executor startup,
and execution, but excludes compilation.

## Metrics and invariants

- `ExecutionStats::executedInstructions` is accumulated per worker by
  `Executor::startExecution()`. Changes to what counts as execution can shift
  every benchmark result even if runtime is unchanged.
- `summarize()` totals instruction counts and durations and records maxima; it
  does not average until report formatting.
- `adjustedRunTime()` subtracts `estimatedStartupTime * trialCount` and clamps
  at zero.
- Time per instruction is computed only after aggregation. A zero instruction
  count is an error.
- Discovery sorts by group and program name, making report order stable.

## Concrete run

The current checkout was built with `cmake --build build -j4`, then sampled
with:

```sh
build/Benchmark --trials 1 --threads 1
```

The report began:

```text
Benchmark root: benchmark/benchmarks
Trials: 1
Threads: 1

arithmetic
  additionLargeNumbers ... instructions=1536
```

Durations are intentionally omitted here because they change with the host and
load. The discovered group/name and instruction count are the useful structural
evidence.

## Choosing workloads

- Put dependency-shape examples under `dependencies`.
- Put broad independent DAGs under `parallel`.
- Put sufficiently large deterministic programs under `longRunning` so startup
  cost is not the dominant measurement.
- Prefer a final reduction or print that prevents the workload from becoming
  disconnected from observable completion.
- Keep inputs deterministic and avoid asserting that more threads are faster.

## Validation after runtime changes

```sh
build/Tests --gtest_filter='Benchmark*:*Benchmark*'
build/Benchmark --trials 1 --threads 1,2,4
```

Check that instruction counts remain stable across worker counts before
interpreting timing. If counts differ, investigate scheduling/completion
semantics first; the timing comparison is not measuring equivalent work.

CI uses one trial at one and four workers to validate the benchmark path within
hosted-runner resource limits. It is not a performance result. Run larger trial
counts or the full worker matrix separately when collecting measurements; the
coarse string workloads intentionally allocate enough data to make repeated
all-worker runs unsuitable for a single correctness-test process.
