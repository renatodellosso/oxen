# Control flow and instruction skipping

`If`, `Else`, `While`, and `Function` all use [`Executor::skipInstruction`](../../../src/interpreter/executor.cpp#L287), but for different reasons: untaken branch, untaken else, false loop iteration, and dormant function template. The method recursively marks block ranges and publishes only non-argument dependencies beyond the skipped range.

This behavior composes with joins and calls:

- skipped branch work still satisfies `BranchMerge`;
- a false `While` skips its body yet releases the loop's stable continuation;
- a skipped return cannot claim `ReturnInvocation` because its marker is indexed;
- a skipped function template never executes, while cloned call bodies start unskipped;
- `While` explicitly unskips its body on a true iteration.

The direct 16-thread run skipped `value = 9`, executed the else loop, and printed `branch-loop=2`. The focused run passed [`ElseBlocksCanContainLoops`](../../../tests/e2e/tests.cpp#L196), and the repository also covers zero iterations and nested completion in [`BranchLoopsCanRunZeroIterations`](../../../tests/e2e/tests.cpp#L206) and [`NestedBlocksPropagateLoopCompletion`](../../../tests/e2e/tests.cpp#L211).

When adding a new control construct, decide separately which nested instructions are marked, which outgoing control edges receive null completion, and which value edges must remain unsatisfied. Treating all dependents alike is unsafe.
