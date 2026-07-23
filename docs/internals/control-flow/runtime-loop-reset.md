# Runtime loop reset

`GoTo` does not move a program counter. In the dependency executor, [`Executor::execSingleInstruction`](../../../src/interpreter/executor.cpp#L627) resets the dependency state for every instruction from the backward target through the `GoTo` and then re-enqueues instructions that are ready.

Under `dependencyStateMutex`, it clears each instruction's `depArgs`, walks in-loop dependent edges, and decrements their `depsFulfilled`. It ignores edges outside the cloned `Subprogram`, outside the loop range, and most disabled edges. A disabled `Call` edge is intentionally considered because call remapping is reconstructed per invocation. `executed` is not reset; it records whether an instruction has ever run, while `queued`, dependency counts, and arguments govern another iteration.

The `While` case only publishes to its immediately following body block while true and suppresses other dependents. When false it skips the body and publishes the one stable post-loop signal. At the end of every instruction, dependents that transitively release a `GoTo` are published last (`releasesGoTo`), ensuring all other outputs are visible before reset begins.

## Evidence

The focused `startExecution.waitsForCallsInsideLoopIterations` run passed with 16 workers; it executes 16 iterations and requires output exactly `0` through `15`. The direct example:

```parallel
void show(int item) { print "loop-call=" + item; }
int i = 0;
while (i < 3) { show(i); i = i + 1; }
```

produced `loop-call=0`, `loop-call=1`, `loop-call=2` in order at 16 threads.

The regression [`LoopBackWaitsForDependencyPublication`](../../../tests/e2e/tests.cpp#L352) stresses three nested loops. Failure modes include stale `depArgs`, dependency counts drifting per iteration, or `GoTo` resetting state while another worker is still publishing results.

## Contributor workflow

A reset change should be checked against three edge classes: an edge wholly inside the loop must replay, an edge entering from before the loop must remain fulfilled, and an edge leaving the loop must not be decremented. An executor-level test should inspect `depsFulfilled` and `depArgs` across at least two iterations. The source-level regression should include a call with a side effect before the loop-back edge and should be repeated in a Release build with 16 workers, because publication/reset races can remain hidden in Debug builds.
