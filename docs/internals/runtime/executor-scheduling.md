# Executor scheduling and dependency publication

The executor schedules a data-flow graph: an instruction becomes runnable only
when every incoming edge has published its result. The implementation is in
[`Executor`](../../../src/interpreter/executor.hpp) and
[`Executor::startExecution`](../../../src/interpreter/executor.cpp); the graph
records live on [`Instruction`](../../../src/instruction.hpp), and the ready
queue is [`ConcurrentQueue`](../../../src/concurrentQueue.hpp).

## Lifecycle

`startExecution()` calls `initScopes()`, then `initQueue()`, starts
`CliArgs::threads` workers, and runs `supervisor()` on the calling thread.
`initQueue()` considers every instruction, so disconnected zero-dependency
instructions can run in parallel.

The readiness state is:

- `depCount`: atomic required-signal count, fixed by parsing and later adjusted
  by call remapping.
- `depsFulfilled`: atomic signals-received count for the current execution or
  loop iteration.
- `depArgs[argIndex]`: value-carrying signals. A dependency without an argument
  index is ordering-only.
- `queued`: prevents duplicate queue entries; `skipped`: suppresses execution.
- `pendingInstructions`: per-worker atomic counts of queued or executing work.
  `supervisor()` stops when their sum reaches zero.

`updateDependency()` stores an argument before incrementing `depsFulfilled`.
When the count reaches `depCount`, it calls `enqueueIfReady()`. A worker clears
`queued` and rechecks both readiness and `skipped` under locks before calling
`execSingleInstruction()`. This second check is necessary because loop reset or
branch skipping may change state after an item enters the queue.

Each `Instruction` owns a `depArgsMutex` that protects writes, resizing, and
clearing of its argument vector. Keeping the mutex beside the vector also gives
each cloned call body independent argument locking even though cloned
instructions reuse local IDs. `depCount` and `depsFulfilled` are atomic, but
the atomics do not by themselves make a multi-field readiness transition
atomic: `dependencyStateMutex` still serializes dependency publication,
`queued`/`skipped` changes, readiness checks, and whole-loop reset. It is
recursive because `skipInstruction()` recursively publishes ordering
dependencies. `pendingInstructions`, the invocation-ID counter, and termination
flags are atomic.

## Invariants and change hazards

1. Publish the argument before the fulfilled count. Otherwise another worker
   may execute with a missing `depArgs` entry.
2. Increment the relevant `pendingInstructions` counter before pushing.
   Otherwise the supervisor can see zero and halt while work is becoming
   visible.
3. All changes that can race with `GoTo` reset must hold
   `dependencyStateMutex`.
4. Lock the destination instruction's `depArgsMutex` whenever mutating or
   clearing its argument vector. Do not select argument locks by instruction
   ID: cloned subprograms reuse local IDs.
5. Keep loop-back edges last in `execSingleInstruction()`. Releasing `GoTo`
   before the instruction's other dependents lets a worker reset the iteration
   while dependency publication is still in flight.
6. A skipped ordering edge must still be fulfilled by `skipInstruction()` or
   downstream joins can deadlock. Argument-indexed edges are deliberately not
   fulfilled with null.
7. Treat an `Executor` and its mutable `Instruction` graph as one execution.
   Reusing it preserves `executed`, dependency, and halt state.

`ExecutionStats` counts dynamic executions in a worker-local counter and sums
them after joining. It is not `program.size()`: loops and cloned calls execute
instructions repeatedly.

## Observed example

The focused test constructs 512 independent `GetLiteral` instructions, runs 16
workers, and checks the aggregate count:

```sh
build/Tests --gtest_filter='startExecution.sumsInstructionCountsFromMultipleWorkers' --gtest_brief=1
```

The same test was included in a 25-test focused documentation-audit run on
2026-07-23:

```text
[==========] 25 tests from 5 test suites ran. (71 ms total)
[  PASSED  ] 25 tests.
```

See
[`startExecution.sumsInstructionCountsFromMultipleWorkers`](../../../tests/interpreter/executor.cpp)
for the graph construction and assertion.

## When changing scheduling

Add a small instruction-level test in
[`tests/interpreter/executor.cpp`](../../../tests/interpreter/executor.cpp), then
an E2E case exercised at every configured thread count in
[`tests/e2e`](../../../tests/e2e). For timing-sensitive paths, use a Release
build and repeat the focused test; one pass is not race evidence.
