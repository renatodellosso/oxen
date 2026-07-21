# Benchmark Architecture

The benchmark executable compiles and runs every Parallel program under
`benchmark/benchmarks`, measures compilation and execution separately, and
reports results for each configured thread count. Benchmark-specific code stays
in this directory; reusable compilation and interpretation live under `src`.

## Execution flow

```text
main
  -> runApplication(argc, argv, stdout, stderr)
       -> parseOptions(arguments)
       -> run(options, benchmark root, output)
            -> discoverPrograms(root)
            -> estimate executor startup with an empty program per thread count
            -> for each program and thread count
                 -> runTrial(program, threads) once per trial
                      -> compileToBytecode(source)
                      -> executeBytecode(bytecode, stats)
                 -> summarize(trial results)
                 -> printProgramSummary(...)
            -> printAggregateSummary(...)
```

`runApplication` is the process boundary. It converts `argv` to string views,
uses the standard `benchmark/benchmarks` root, and converts exceptions into a
diagnostic and a nonzero exit code. `run` accepts its root and output stream as
arguments so the workflow can be exercised in-process by tests.

## Modules

- `main.cpp` is the minimal executable entry point.
- `application.*` coordinates discovery, trials, aggregation, and reporting.
- `options.*` parses `--trials` and `--threads`. Values must be positive
  integers, and malformed comma-separated thread lists are rejected.
- `discovery.*` finds `.ox` files one directory below the benchmark root. The
  directory name becomes the group, and results are sorted by group and program
  name for deterministic output.
- `runner.*` performs one compile-and-execute trial. It records steady-clock
  durations and collects the executor's instruction count. Program output is
  redirected during execution so it does not contaminate the report.
- `results.*` reduces a set of trials into totals and maxima without performing
  I/O or measuring time.
- `report.*` formats headers, per-program summaries, and the per-thread and
  overall nanoseconds-per-instruction results to a supplied stream.
- `types.hpp` contains the data passed between these modules.

## Shared compiler and interpreter paths

The benchmark does not assemble compiler or executor internals itself.
`compileToBytecode` in `src/compiler` compiles a source stream into an in-memory
string. `executeBytecode` in `src/interpreter` parses that string, transfers the
instruction buffer to a `Subprogram`, executes it, and optionally updates
`ExecutionStats`.

The CLI uses the same paths. This keeps benchmark behavior aligned with normal
compile-and-run behavior and avoids copying bytecode and parsed instruction
vectors between stages.

`ScopedStreamRedirect` in `src/streamRedirect.hpp` provides exception-safe
stream restoration. The benchmark uses it only to suppress output produced by
the measured program.

## Data and timing

Each `TrialResult` contains compile time, run time, and executed instructions.
`ProgramSummary` stores totals and maxima for one program/thread-count pair.
`AggregateSummary` contains total execution time and instruction count across
the entire run, plus the corresponding totals for each requested thread count.

Compilation timing covers source compilation into bytecode. Execution timing
covers bytecode parsing, program construction, executor startup, and execution;
it does not include compilation or report formatting. Before running the
programs, the benchmark executes an empty program for every requested thread
count and averages the configured number of trials to estimate per-trial
startup. Each time-per-instruction value subtracts that thread count's estimated
startup for every measured trial from aggregate run time, clamps the adjusted
time to zero, and divides by aggregate executed instructions. The overall value
sums the adjusted per-thread runtimes. A run that executes no instructions is
treated as an error.

Programs in `benchmarks/longRunning` exercise arithmetic, branching, and
independent control-flow chains for long enough to amortize executor startup.
The regression suite requires at least two programs in this group and at least
250,000 dynamically executed bytecode instructions from each program with one
worker. The instruction threshold keeps this contract deterministic across
machines; elapsed time remains a measured result rather than a test assertion.

Programs in `benchmarks/parallel` focus on workloads that expose useful DAG
width: independent control-flow lanes perform the substantial work and join
only when their results are reduced. The group covers flat arithmetic and
branch-heavy fan-out, heterogeneous lanes, a wide fork/join reduction, and
coarse string workloads. The bulk, growth, and doubling programs make each
independent operation large enough to amortize instruction scheduling and are
intended to demonstrate multi-worker speedup in a Release build. Speedup is
reported by the benchmark rather than asserted by tests because timing depends
on the host and its current load. The balanced string quadrupling and
quintupling programs are sized to saturate roughly four workers before memory
bandwidth and executor synchronization outweigh additional parallelism.

## Errors and tests

Internal benchmark failures use exceptions with relevant paths or exit codes.
Only `runApplication` catches them for command-line presentation. Compiler and
interpreter failures retain their CLI `ExitCode` behavior. The shared `Error`
type lets the benchmark preserve interpreter diagnostics while carrying the
corresponding exit code to an application boundary.

Tests live under `tests/benchmark`. Unit tests cover parsing, discovery,
aggregation, and formatting using deterministic inputs. Integration tests use
temporary benchmark trees and invoke the workflow directly with one trial and
one thread. CMake links every benchmark source except `main.cpp` into the test
binary, allowing tests to call the same implementation used by the benchmark
executable.
