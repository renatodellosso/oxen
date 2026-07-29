# Executor concurrency and control-flow mutation

Branches and loops mutate instruction state while workers are concurrently
publishing dependencies. The critical code is
[`Executor::skipInstruction`, `updateDependency`, and the `While`/`GoTo` cases](../../../src/interpreter/executor.cpp),
with state declared in [`Executor`](../../../src/interpreter/executor.hpp) and
[`Instruction`](../../../src/instruction.hpp).

## Branch mutation

`If` and `Else` use [`valToBool`](../../../src/value.cpp). The untaken path's
following `Block` is passed to `skipInstruction()`. For a block, the method
first marks the complete nested range skipped, then recursively processes it.
This two-pass order prevents a dependency released by an early skipped
instruction from queuing a later instruction that has not yet been marked.

Skipped instructions publish only non-argument ordering edges leaving the
skipped region. This lets `BranchMerge` and downstream control flow complete
without manufacturing values for computations that did not run.

## Loop mutation

When `While` is true, it releases only its immediately following body block and
does not publish its exit dependents. When false, it skips the body and publishes
the stable loop-completion signal.

At `GoTo`, the executor holds `dependencyStateMutex`, identifies the backward
range, and resets state for edges that will be replayed. The reset protocol
requires clearing each instruction's `depArgs` under that instruction's own
mutex and decrementing its atomic fulfilled count while the outer state mutex
is held. It then requeues instructions already ready for the next iteration.
External, before-loop, after-loop, and most disabled edges are excluded from
reset.

`execSingleInstruction()` publishes ordinary dependents first and edges that
eventually release a `GoTo` last. `releasesGoTo()` follows nested
`CallCompletion` wrappers, so the rule also covers a loop-back signal mediated
by a function call.

## Locking model and hazards

- `dependencyStateMutex` must cover both graph-wide reset and dependency
  publication, including compound comparisons of the atomic dependency counts.
  Narrowing it reintroduces reset/publication races.
- The mutex is recursive because a branch skip can recurse and call
  `updateDependency()`.
- Every `Instruction` owns its `depArgsMutex`; argument-vector mutation must
  lock the destination instruction rather than select a lock by ID. Cloned call
  bodies therefore receive independent locks even though they reuse local IDs.
- `depCount` and `depsFulfilled` are atomic because `Instruction` instances are
  shared across workers. Their copy and move operations snapshot the counts;
  they do not transfer an instruction's mutex.
- Never enqueue `GoTo` until all sibling dependents from the completing
  instruction have been published.
- Reset must clear argument slots as well as counts; stale values can make a
  later iteration observe a previous result.
- Do not reset `Instruction::executed`; it records whether an instruction ever
  ran, not whether it ran in the current iteration.

## Observed example

This example calls a side-effecting function in each loop iteration:

```ox
int total = 0;
void add(int item) {
  total = total + item;
  print "added " + item;
}
int i = 1;
while (i <= 3) {
  add(i);
  i = i + 1;
}
print "total=" + total;
```

With the example saved as `loop-call.ox`, the following command uses eight workers:

```sh
build/CLI -t loop-call.ox -h 8
```

```text
added 1
added 2
added 3
total=6
```

The exact sequence demonstrates that each call's print/write completed before
the loop advanced. The stronger targeted command repeated loop-call, call
completion, and concurrent-return cases 20 times:

```sh
build/Tests --gtest_filter='startExecution.waitsForCallsInsideLoopIterations:startExecution.isolatesBranchedReturnsAcrossConcurrentCallInvocations:startExecution.waitsForTerminalCallSideEffectsAfterDependencyRemapping' --gtest_repeat=20 --gtest_brief=1
```

All 60 test executions passed on 2026-07-23. The nested-loop E2E regression
[`LoopBackWaitsForDependencyPublication`](../../../tests/e2e/tests.cpp) is the
smallest source-level guard for publication-before-reset ordering.
