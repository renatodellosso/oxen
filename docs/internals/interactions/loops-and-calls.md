# Loops and calls

Every source loop already contains a generated call to `_body`; user calls inside the original body therefore become nested calls in a cloned generated-body invocation. [`GraphLinker::processExpression`](../../../src/compiler/graphLinker.cpp#L338) remaps `GoTo` dependencies across callee last uses and redirects repeatable loop work to `While`. At runtime, call completion barriers ensure `GoTo` cannot reset the iteration until cloned call side effects finish.

Example executed at 16 threads:

```parallel
void show(int item) { print "loop-call=" + item; }
int i = 0;
while (i < 3) { show(i); i = i + 1; }
```

Observed output was exactly `loop-call=0`, `loop-call=1`, `loop-call=2`. Debug bytecode showed the generated `_body`, `While`, `Call`, and `GoTo -8` sequence. The focused tests [`waitsForCallsInsideLoopIterations`](../../../tests/interpreter/executor.cpp#L551) and 16-worker [`FunctionsCanBeCalledInsideLoops`](../../../tests/e2e/tests.cpp#L593) passed.

The executor test was also part of a 20-repeat concurrency run with branched-return, side-effect-barrier, and recursive-call regressions; all 80 executions passed.

Invariant: each iteration's call invocation is fully wired and its ordering-relevant terminal work completes before loop reset. A direct call result is not sufficient if the callee has later side effects.

## Contributor workflow

A loop/call change should validate the serialized `_body` call remaps, invocation-local completion barriers, and publication-before-`GoTo` ordering. The focused executor test should assert both ordered output and that reusable `Call` dependents remain enabled after execution. The E2E matrix should include a void call, a value-returning call, and a nested call inside the loop at 1 and 16 workers; Release-mode repetition is required before a concurrency-sensitive change is considered stable.
