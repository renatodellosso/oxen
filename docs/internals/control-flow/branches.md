# Branch lowering and execution

[`IfExpression`](../../../src/compiler/expression.hpp#L113) owns the condition, then block, optional `Else` instruction and else block, plus an unconditional `BranchMerge`. Its serialization order is implemented by [`IfExpression::toByteCode`](../../../src/compiler/expression.cpp#L309). [`IfExpression::linkInternally`](../../../src/compiler/expression.cpp#L352) links the condition to `If`, both branch completions to the merge, and—when present—the `If` plus then completions to `Else`.

At runtime, [`Executor::execSingleInstruction`](../../../src/interpreter/executor.cpp#L345) handles `If` by skipping the next block when false. `Else` skips its next block when the original condition was true. `BranchMerge` is a no-op whose dependency count is the join barrier. Skipping publishes non-argument dependencies, so the untaken path still contributes its merge signals.

Invariant: exactly one branch executes, but both paths satisfy the merge's fixed dependency graph. Code after an `if` must depend on the merge, not directly on one branch. [`GraphLinker::processExpression`](../../../src/compiler/graphLinker.cpp#L338) also chains a following top-level statement after a prior branch merge.

## Example and observed run

```parallel
int value = 0;
if (false) value = 9;
else { while (value < 2) value = value + 1; }
print "branch-loop=" + value;
```

With the example saved as `example.ox`, `build/CLI -t example.ox -h 16` produces `branch-loop=2`. `build/CLI -t example.ox -c -d -o <bytecode>` shows `If` instruction 6, `Else` 11, and `BranchMerge` 30. The structural tests [`linksFollowingStatementsToIfBranchMerge`](../../../tests/compiler/graphLinker.cpp#L498) and [`IfStatementsCanRunInsideElseBlocks`](../../../tests/e2e/tests.cpp#L638) also pass.

Typical bugs are executing both branches, publishing a skipped argument value, or linking a continuation directly to a repeatable instruction nested in a branch.

## Contributor workflow

A lowering change should preserve the bytecode order `If`, then block, optional `Else`, else block, and `BranchMerge`. The maintainer can inspect that order with the debug-bytecode command above, add an `IfExpression` unit test for exact edges, and add an E2E program covering both condition values at 1 and 16 workers. A continuation after the branch must have a dependency on `BranchMerge`; no continuation edge should target an instruction that runs only on one path.
