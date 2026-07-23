# Call completion barriers

A call result and a completed invocation are intentionally different. A return can satisfy `print f()` while later statements in `f` are still running. Resource users and sequencing edges that must wait for those statements use [`CallCompletion`](../../../src/instruction.hpp): an invocation ID, atomic remaining-signal count, final dependent, and optional result.

The executor identifies terminal instructions with [`getCompletionInstructionIds`](../../../src/interpreter/executor.cpp). An instruction is terminal only if it has no active dependent inside the cloned body; a nested `Call` with an internal continuation is therefore not terminal. Function declarations skip their stored bodies during the scan. [`getStableCompletionInstructionIds`](../../../src/interpreter/executor.cpp) replaces repeating loop-region terminals with the loop's one-shot `While` signal.

In the `Call` case, `attachCompletionBarrier` adds one wrapper edge per unique signal. [`updateDependency`](../../../src/interpreter/executor.cpp) decrements `remaining` and releases the stored dependent exactly at zero, preserving any result observed along the way.

## Evidence

The terminal-side-effect regression repeatedly applies this pattern:

```parallel
int value = 0;
void printValue() { print value; }
printValue();
value = value + 1;
printValue();
```

The first invocation's terminal `Print` must signal completion before the
assignment publishes the next value, so the required output is `0` followed by
`1` even with 16 workers.

The focused run passed [`waitsForTerminalCallSideEffectsAfterDependencyRemapping`](../../../tests/interpreter/executor.cpp): eight calls to `printValue()` alternate with writes and must print exactly `0` through `7` at 16 threads. [`waitsForLeadingSideEffectsBeforeRecursiveCalls`](../../../tests/interpreter/executor.cpp) also passed, requiring recursive countdown output `12` through `0` in order.

The four concurrency-focused executor regressions (branched returns, terminal side effects, calls in loops, and recursive leading effects) were also repeated 20 times in the current build: 80/80 test executions passed.

If terminal discovery includes nested calls with internal continuations, callers release early. If loop terminals are not collapsed, barriers either over-count forever or release from a repeated iteration.

## Change workflow

A contributor changing terminal discovery should trace the cloned instruction
IDs returned by `getCompletionInstructionIds()` before changing barrier counts.
The focused executor tests named above cover ordinary and recursive calls; a
loop-related change also requires
[`waitsForCallsInsideLoopIterations`](../../../tests/interpreter/executor.cpp).
The affected tests should run repeatedly in a Release build with multiple
workers because premature release is timing-sensitive.
