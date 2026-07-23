# Output serialization

`Print` is a runtime side effect handled by the `InstructionType::Print` branch
of [`Executor::execSingleInstruction`](../../../src/interpreter/executor.cpp).
It converts the dependency value with [`valToStr`](../../../src/value.cpp),
appends a newline, locks `Executor::coutMutex`, and performs one `std::cout`
insertion.

The lock guarantees that workers belonging to one `Executor` do not splice
characters from different print instructions into a line. It does **not** make
independent prints execute in source order. Graph ordering must come from
resource dependencies, not the output mutex.

## Compiler participation

In [`GraphLinker::linkExpression`](../../../src/compiler/graphLinker.cpp), a
`Print` calls `Expression::redirectResourceCompletionsTo(expr)`. If the printed
expression completes a read of a resource, later writers wait for the `Print`
rather than only for value computation. This prevents a shared value from being
overwritten before its side effect consumes it.

Calls require another layer: `getCompletionInstructionIds()`,
`getStableCompletionInstructionIds()`, and `CallCompletion` in
[`executor.cpp`](../../../src/interpreter/executor.cpp) ensure terminal prints
inside a function can participate in the call's completion barrier.

## Invariants and change hazards

1. Format the entire line before acquiring `coutMutex`, then emit it as one
   insertion while holding the lock.
2. Never use this mutex as a dependency mechanism. It protects bytes, not
   semantic ordering.
3. Preserve redirected resource-completion edges when changing print lowering.
   Otherwise `print value; value = ...;` may print the later value.
4. Preserve terminal print signals when changing call completion discovery.
   Otherwise a caller may mutate a captured value while an earlier call still
   plans to print it.
5. `coutMutex` is per executor. Concurrent executor instances do not share it.
6. Tests and benchmarks redirect global `std::cout`; do not run such redirects
   concurrently in the same process.

## Observed example

```ox
print "alpha";
print "beta";
print "gamma";
```

```sh
build/CLI -t /tmp/parallel-runtime-docs-output-20260723.ox -h 16
```

One observed ordering on 2026-07-23 was:

```text
gamma
alpha
beta
```

Every line is intact, while ordering differs from source order because the
three graphs are independent. This is why
[`PrintWorksWithMultiplePrints`](../../../tests/e2e/tests.cpp) uses an unordered
expectation. By contrast,
[`waitsForTerminalCallSideEffectsAfterDependencyRemapping`](../../../tests/interpreter/executor.cpp)
asserts exact order where shared-resource dependencies require it.
