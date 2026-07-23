# Branches and loops

A lowered loop contains instructions that publish every iteration, so [`BlockExpression::completionExpression`](../../../src/compiler/expression.hpp#L153) points to the `While`. When that block is nested in an `IfExpression`, [`BlockExpression::getCompletionExpressions`](../../../src/compiler/expression.cpp#L251) returns only the stable `While`; the branch merge must not depend on the condition expression, body, call, or `GoTo`.

At runtime a false branch recursively skips the loop block. If the branch is taken but the loop condition is initially false, `While` skips its iteration block and publishes its stable completion, allowing the branch merge to finish.

The direct example `if (false) ... else { while (value < 2) ... }` printed `branch-loop=2` at 16 threads. The focused E2E [`ElseBlocksCanContainLoops`](../../../tests/e2e/tests.cpp#L196) passed at 16 workers. [`mergeDoesNotDependOnRepeatableElseLoopInstructions`](../../../tests/compiler/expression.cpp#L122) asserts the exact completion-edge invariant, while [`BranchLoopsCanRunZeroIterations`](../../../tests/e2e/tests.cpp#L206) covers the false-first-condition path.

Failure modes are an over-fulfilled merge after several iterations, a merge that never receives a zero-iteration signal, or a skipped nested block retaining queued work.
