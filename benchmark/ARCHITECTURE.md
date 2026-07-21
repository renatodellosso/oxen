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
- `report.*` formats headers, per-program summaries, and the aggregate
  nanoseconds-per-instruction result to a supplied stream.
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
the entire run.

Compilation timing covers source compilation into bytecode. Execution timing
covers bytecode parsing, program construction, executor startup, and execution;
it does not include compilation or report formatting. The final
time-per-instruction value is aggregate run time divided by aggregate executed
instructions. A run that executes no instructions is treated as an error.

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
