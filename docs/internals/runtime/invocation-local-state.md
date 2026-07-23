# Invocation-local and reusable state

Function declarations are reusable, but each call must own its mutable
execution state. The relevant types are
[`Function`](../../../src/interpreter/function.hpp),
[`Subprogram`](../../../src/interpreter/subprogram.hpp),
[`Instruction`, `InstrDependent`, `ReturnInvocation`, and `CallCompletion`](../../../src/instruction.hpp),
and the `InstructionType::Call` branch in
[`Executor::execSingleInstruction`](../../../src/interpreter/executor.cpp).

## Reusable declaration state

Executing a `Function` instruction constructs a `Function` from the following
block. `Function::Function()` copies that block into a stored `Subprogram` and
records name, return type, and `generatedLoopBody`. That stored body is a
template. The declaration's function `Value` can be read by many calls.

At each `Call`, `func->getBody().clone()` creates a new instruction vector,
rewrites internal dependent pointers, and calls `setSubprogramPointers()`.
The cloned block receives a fresh child `Scope<Value>`. Argument declaration
scopes and dependencies are then attached before `enqueueIfReady(block)` makes
the invocation visible.

## Per-invocation objects

- `nextCallInvocationId.fetch_add(1)` supplies a diagnostic identity.
- `ReturnInvocation` owns an atomic `claimed` latch and a snapshot of the
  caller destinations. Every cloned `Return` points to it. The first executed
  return wins; later returns cannot overwrite another invocation's result.
- `CallCompletion` owns an atomic signal count, one caller dependent, and the
  first non-null result seen. Terminal instructions and remapped resource
  completions signal it, and it releases the caller exactly once.
- The cloned instructions own invocation-specific `depArgs`, counts, skipped
  state, scopes, and dynamically attached edges.

The pointer fields on `InstrDependent` are wrappers into invocation state.
Persistent call-template edges may describe where a call ultimately leads, but
must not store a mutable winning-return latch or a completion counter shared by
different calls.

## Invariants and change hazards

1. Finish all remapping before enqueuing the cloned block.
2. Keep parameters in the cloned block scope. Sharing declaration scopes lets
   simultaneous calls overwrite one another's arguments.
3. Snapshot return destinations into `ReturnInvocation`; do not read a call's
   mutable `dependents` later and assume it still describes the same invocation.
4. Preserve pointer identity when cloning. `Subprogram` rewrites only edges
   whose target lies inside the copied range and leaves external targets as
   external pointers; call handling then adds the invocation-specific edges.
5. Do not mutate reusable dependency edges with per-call counters or selected
   results. The failure mode is nondeterministic cross-talk between concurrent
   calls.
6. Completion is broader than return selection. A call can produce its return
   and still have terminal side effects that must complete before resource
   dependents proceed.

## Observed example

The executor regression launches 12 branched calls through 16 workers and
verifies that each invocation independently returns `outer`, `inner`, or
`fallback`. Together with call-side-effect and loop-call regressions it was
repeated 20 times:

```sh
build/Tests --gtest_filter='startExecution.waitsForCallsInsideLoopIterations:startExecution.isolatesBranchedReturnsAcrossConcurrentCallInvocations:startExecution.waitsForTerminalCallSideEffectsAfterDependencyRemapping' --gtest_repeat=20 --gtest_brief=1
```

Observed on 2026-07-23:

```text
# repeated 20 times
[==========] 3 tests from 1 test suite ran. (24 ms total)
[  PASSED  ] 3 tests.
```

The precise source assembly and unordered-result assertion are in
[`isolatesBranchedReturnsAcrossConcurrentCallInvocations`](../../../tests/interpreter/executor.cpp).
