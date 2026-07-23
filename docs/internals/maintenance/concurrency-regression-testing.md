# Concurrency regression testing

Concurrency fixes require a focused reproducer, diagnostics that identify the
invocation or iteration, and repeated Release-mode validation. A single passing
run shows only that one schedule worked.

## Build the configuration that exposes scheduling bugs

CI tests Release builds on Ubuntu, Windows, and macOS. Reproduce locally with a
separate directory so a Debug build cannot mask timing:

```sh
cmake -B build-ci -DCMAKE_BUILD_TYPE=Release
cmake --build build-ci -j8
```

Use the repository's normal `build/Tests` for fast development, then repeat the
reproducer with `build-ci/Tests` before considering the fix verified.

## Start with the smallest layer

1. Add an executor unit test in
   [`tests/interpreter/executor.cpp`](../../../tests/interpreter/executor.cpp)
   when the bad graph can be assembled directly. This gives deterministic
   control over edges, disabled dependents, and cloned call bodies.
2. Add a graph-linker test in
   [`tests/compiler/graphLinker.cpp`](../../../tests/compiler/graphLinker.cpp)
   when compilation created the wrong dependency.
3. Add an E2E program in
   [`tests/e2e/tests.cpp`](../../../tests/e2e/tests.cpp) to preserve the complete
   source-to-runtime behavior. The parameterization in
   [`e2e.cpp`](../../../tests/e2e/e2e.cpp) runs it with 1, 2, 4, 8, and 16
   workers.

Prefer `ExpectOrdered` only when the dependency graph promises output order.
Independent output belongs in `ExpectUnordered`; forcing an order would test a
property the language intentionally does not provide.

## Useful diagnostics

Verbose execution (`build/CLI -t program.ox -e -h 16`) already logs instruction
construction, queue activity, invocation IDs, and call-completion signal counts
through [`logging.hpp`](../../../src/logging.hpp). New diagnostics should include
the call invocation ID, instruction ID/type, remaining barrier count, and loop
iteration owner. Build the whole message before writing it so output from
multiple workers is not fragmented.

Avoid pointer addresses as the only correlation key. They identify an object in
one process but do not distinguish the intended semantic invocation in a
repeatable report.

## Stress commands

```sh
build-ci/Tests \
  --gtest_filter='startExecution.isolatesBranchedReturnsAcrossConcurrentCallInvocations' \
  --gtest_repeat=1000 --gtest_break_on_failure

build-ci/Tests \
  --gtest_filter='E2E/E2EFixture.E2E/FunctionsCanBeCalledInsideLoops16:E2E/E2EFixture.E2E/CallsAreSequencedCorrectly16' \
  --gtest_repeat=500 --gtest_break_on_failure
```

Tune repetition counts downward during investigation, but record the final
count and build type in the change description.

## Concrete verification from this documentation pass

The development build ran the executor reproducer plus the three 16-worker E2E
paths for call ordering, calls in loops, and first-return selection:

```text
[==========] Running 8 tests from 6 test suites.
[  PASSED  ] 8 tests.
```

The complete command is recorded in
[test architecture](test-architecture.md#cross-layer-verification-example).
This is a smoke result, not a stress-test claim.

## Completion criteria

- the new test fails without the fix for the expected reason;
- relevant verbose logs explain the wrong ordering or state ownership;
- the focused Release test survives repeated multi-worker runs;
- the full Release suite passes;
- Debug results are reported separately;
- CI status is stated explicitly rather than inferred from local success.

