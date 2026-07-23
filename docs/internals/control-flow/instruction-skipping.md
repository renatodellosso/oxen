# Instruction skipping

[`Executor::skipInstruction`](../../../src/interpreter/executor.cpp#L287) represents untaken control flow without rewriting the graph. For a `Block`, it first marks every contained instruction as skipped—including nested blocks as ranges—then recursively processes them. Marking first is essential: publishing skipped dependencies must not accidentally enqueue another instruction inside the same skipped block.

After marking, the method publishes `nullptr` through every non-argument dependent outside the skipped range. Argument-indexed edges are intra-expression value flow and are deliberately not fulfilled. This distinction is particularly important for skipped `Return` instructions: their invocation marker retains an argument index, so an untaken return cannot claim a call with a null result.

Passing `markSkippedAs=false` unskips a loop body before a new iteration. It does not publish dependencies. `Function` declaration execution also skips its stored body because that bytecode is a template; `Call` executes a clone instead.

Invariant: skipping suppresses values and side effects, yet satisfies control/sequencing edges that a join is waiting for. A skipped instruction must never be queued.

## Evidence

In the direct run, `if (false) value = 9; else ...` left the assignment skipped and ultimately printed `branch-loop=2`. The current 16-worker tests [`SkippedIfBranchPreservesPreviousValue`](../../../tests/e2e/tests.cpp#L191), [`BranchLoopsCanRunZeroIterations`](../../../tests/e2e/tests.cpp#L206), and [`IfStatementsCanRunInsideElseBlocks`](../../../tests/e2e/tests.cpp#L638) cover skipped writes, skipped loop bodies, and nested branch propagation.

Common failures are publishing indexed null arguments, only marking the outer block, or releasing a merge before the taken branch completes.

## Contributor workflow

A new skippable construct must define its skipped instruction range and distinguish value edges from ordering edges. The executor unit test should build an `Instruction` graph with both an indexed argument dependent and an unindexed external dependent, then verify that only the external dependent is fulfilled. The matching E2E case should place the construct inside a false branch and inside a zero-iteration loop; repeated 16-worker runs are appropriate when the change can queue work while `skipInstruction()` holds `dependencyStateMutex`.
