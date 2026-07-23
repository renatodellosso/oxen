# CLI, tests, and benchmark integration

The CLI, E2E suite, and benchmark intentionally converge on the same production
compiler and interpreter paths. Build target assembly is in
[`CMakeLists.txt`](../../../CMakeLists.txt); application entry points are
[`src/main.cpp`](../../../src/main.cpp) and
[`benchmark/main.cpp`](../../../benchmark/main.cpp).

## Which path each consumer exercises

| Consumer | Entry point | Production path |
|---|---|---|
| `build/CLI` | `runCli()` | validates files, then `executeCommand()` |
| parameterized E2E | `executeCommand(args)` | compile-and-interpret using a temporary `.ox` file |
| compiler/interpreter unit integration | `compileToBytecode()` + `executeBytecode()` | in-memory source and bytecode streams |
| `build/Benchmark` | `runApplication()` -> `run()` -> `runTrial()` | same in-memory APIs, plus timing/stats |

[`tests/e2e/e2e.cpp`](../../../tests/e2e/e2e.cpp) instantiates every `E2eTest` at
1, 2, 4, 8, and 16 threads and captures `std::cout`. These tests validate exit
code and output semantics but do not exercise CLI parsing.
[`tests/cli.cpp`](../../../tests/cli.cpp) separately covers parsing and
validation.

[`benchmark/runner.cpp`](../../../benchmark/runner.cpp) times compilation and
execution separately. `runSource()` suppresses program output with
[`ScopedStreamRedirect`](../../../src/streamRedirect.hpp), calls
`executeBytecode()` with `ExecutionStats`, and preserves typed execution errors
while adding the source path. `runEmptyTrial()` estimates executor startup.

[`benchmark/application.cpp`](../../../benchmark/application.cpp) discovers
programs, runs each requested thread count and trial, and aggregates results.
[`benchmark/report.cpp`](../../../benchmark/report.cpp) subtracts estimated
startup per trial, clamps adjusted time to zero, and reports nanoseconds per
dynamically executed instruction. Speedup is observed, never asserted, because
wall time depends on the host. See
[`benchmark/ARCHITECTURE.md`](../../../benchmark/ARCHITECTURE.md) for benchmark
module details.

## Invariants and change hazards

- New source behavior needs focused compiler/executor coverage and an E2E case;
  CLI argument behavior needs a CLI test.
- Keep reusable compile/execute logic in `src`. Benchmark-only code belongs in
  `benchmark`, and the test target links benchmark sources except `main.cpp`.
- `ExecutionStats::executedInstructions` is dynamic work. Preserve equal counts
  across thread counts; scheduling parallelism must not duplicate or omit work.
- Output capture redirects process-global `std::cout`. Avoid concurrently
  running in-process workflows that install separate redirects.
- Benchmark execution timing includes bytecode parsing, program construction,
  worker startup, and execution, but excludes compilation and report output.
- Never turn timing or speedup into a deterministic test assertion. Use
  instruction thresholds for workload-size contracts.
- Use a Release build for performance claims and race stress. A passing Debug
  E2E matrix is useful functional coverage, not concurrency proof.
- CMake currently discovers both `src` and tests recursively and fetches
  GoogleTest from `main`; changes to layout or upstream GoogleTest can alter the
  build surface.

## Observed examples

Shared API and instruction-count integration:

```sh
build/Tests --gtest_filter='BenchmarkFixture.SharedExecutionCompilesAndCollectsStatistics:BenchmarkFixture.CountsTheSameInstructionsAcrossThreadCounts' --gtest_brief=1
```

Current output for that command:

```text
[==========] 2 tests from 1 test suite ran.
[  PASSED  ] 2 tests.
```

A real one-thread benchmark run:

```sh
build/Benchmark --trials 1 --threads 1
```

The run prints one row per benchmark and aggregate rows for the requested
thread counts. In the current benchmark set, the stable aggregate count is:

```text
total_executed_instructions=259287
```

Timing fields are host-dependent; the successful workflow and instruction
count are the reproducible facts as long as the benchmark programs do not
change.
