# Test architecture and coverage map

The test binary links production compiler/interpreter sources and all benchmark
sources except executable entry points. `CMakeLists.txt` requests CTest
registration with `gtest_discover_tests()`, while direct execution through
`build/Tests` supports filters and repetition. Check registration in the active
build. In the documentation audit's existing build,
`ctest --test-dir build -N` reported zero tests, so `build/Tests` was the
reliable full-suite command.

## Test layers

| Layer | Location | Protects |
|---|---|---|
| Small utilities | `tests/*.cpp` | scopes, queues, logging, CLI parsing, errors |
| Compiler units | `tests/compiler` | tokens, AST shape, expression completion, graph edges, deferred linking, serialized bytecode |
| Interpreter units | `tests/interpreter` | bytecode parsing and deliberately assembled execution graphs |
| End to end | `tests/e2e` | source compilation and execution with 1, 2, 4, 8, and 16 workers |
| Benchmark units/integration | `tests/benchmark` | discovery, options, reporting, shared execution APIs, bundled workloads |

The E2E fixture writes a temporary `.ox` file, calls `executeCommand()`, captures
`std::cout`, and delegates validation to `ExpectOrdered`, `ExpectUnordered`, or
`ExpectError`. See [`e2e.cpp`](../../../tests/e2e/e2e.cpp) and
[`expectations.cpp`](../../../tests/e2e/expectations.cpp).

## Invariant-to-test map

| Invariant | Smallest focused coverage | Full-path coverage |
|---|---|---|
| expression operands keep argument indices | `tests/compiler/expression.cpp` | arithmetic and comparison E2E cases |
| resource reads/writes serialize correctly | `tests/compiler/graphLinker.cpp` | variable update and call sequencing cases |
| branches merge resource state | `linkGraph.linksElseBranchesFromPreBranchResourceState` | branch write and nested branch cases |
| loops expose stable completion | expression/compiler tests | nested loops and loops inside branches |
| call invocations do not share return state | executor call tests | repeated calls and multiple-return cases |
| calls wait for terminal side effects | executor completion tests | calls in loops and call sequencing cases |
| worker count does not change semantics | executor statistics tests | every parameterized E2E case |
| benchmark uses production execution | `BenchmarkFixture.SharedExecutionCompilesAndCollectsStatistics` | bundled benchmark regression suite |

## Cross-layer verification example

This command was run after building the current checkout:

```sh
build/Tests --gtest_filter='Tokenizer.identifiesMixOfTypes:AstBuilder.buildsWhileStatements:linkGraph.linksElseBranchesFromPreBranchResourceState:startExecution.isolatesBranchedReturnsAcrossConcurrentCallInvocations:E2E/E2EFixture.E2E/CallsAreSequencedCorrectly16:E2E/E2EFixture.E2E/FunctionsCanBeCalledInsideLoops16:E2E/E2EFixture.E2E/ReturnsUseFirstExecutedReturnStatement16:BenchmarkFixture.SharedExecutionCompilesAndCollectsStatistics'
```

Observed result:

```text
[==========] Running 8 tests from 6 test suites.
[  PASSED  ] 8 tests.
```

It deliberately samples one test from each important layer. It does not replace
`build/Tests` or repeated Release testing.

## Adding coverage

- Demonstrate a bug before changing production code.
- Assert graph structure directly when the defect is a missing or extra edge;
  output alone makes such failures harder to diagnose.
- Give executor tests enough independent work to expose the race while keeping
  their expected dependency graph readable.
- Add only one E2E entry per distinct language program: thread-count coverage is
  supplied automatically by `testing::Combine()`.
- Test both positive behavior and the corresponding error boundary.
- Keep timing out of correctness assertions. Instruction counts can be stable;
  elapsed time is machine-dependent.

## Running tests

The following commands build the active tree, run the full suite, and select
focused compiler and 16-worker E2E cases:

```sh
cmake --build build
build/Tests
build/Tests --gtest_filter='linkGraph.*'
build/Tests --gtest_filter='E2E/E2EFixture.E2E/*16'
```

The optional [`coverage.sh`](../../../coverage.sh) uses Clang source-based
coverage on macOS. It is a local reporting workflow, not a correctness gate.
