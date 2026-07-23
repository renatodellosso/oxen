# Branches and loops

A lowered loop contains instructions that publish every iteration, so [`BlockExpression::completionExpression`](../../../src/compiler/expression.hpp#L153) points to the `While`. When that block is nested in an `IfExpression`, [`BlockExpression::getCompletionExpressions`](../../../src/compiler/expression.cpp#L251) returns only the stable `While`; the branch merge must not depend on the condition expression, body, call, or `GoTo`.

At runtime a false branch recursively skips the loop block. If the branch is taken but the loop condition is initially false, `While` skips its iteration block and publishes its stable completion, allowing the branch merge to finish.

For example, the following source executes the loop only through the `else` path:

```parallel
int value = 0;
if (false) value = 9;
else { while (value < 2) value = value + 1; }
print value;
```

The example printed `2` at 16 workers. The focused E2E [`ElseBlocksCanContainLoops`](../../../tests/e2e/tests.cpp#L196) covers the same interaction. [`mergeDoesNotDependOnRepeatableElseLoopInstructions`](../../../tests/compiler/expression.cpp#L122) asserts the exact completion-edge invariant, while [`BranchLoopsCanRunZeroIterations`](../../../tests/e2e/tests.cpp#L206) covers the false-first-condition path.

Failure modes are an overfulfilled merge after several iterations, a merge that never receives a zero-iteration signal, or a skipped nested block retaining queued work.

## Contributor workflow

A change should retain one merge dependency for the loop as a whole, regardless of iteration count. The expression test should reject dependencies from `BranchMerge` to the generated body `Call` or `GoTo`; E2E coverage should exercise a skipped branch, an initially false loop, and a multi-iteration loop at both 1 and 16 workers.
