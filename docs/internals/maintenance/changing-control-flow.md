# Adding or changing control flow

Control flow is a compiler/runtime protocol, not only an executor branch. A
construct must define which instructions run, which instructions are skipped,
what represents completion, how resource state is merged, and whether the
construct can execute repeatedly.

## Design questions to answer first

1. What expression owns the construct, and in what order does
   `getWithSubExpressions()` emit its condition, bodies, markers, and merge?
2. What instruction is the stable completion signal returned by
   `getCompletionExpressions()`?
3. Which resource state exists at each branch entry and after convergence?
4. How does `Executor::skipInstruction()` satisfy edges from work that will not
   execute?
5. If execution repeats, which fields must be reset and which dependency edges
   must remain immutable?
6. Can a nested call or loop still be running when the apparent final
   instruction executes?

## Compiler checklist

- Build an explicit expression shape in
  [`astBuilder.cpp`](../../../src/compiler/astBuilder.cpp). Existing `if`
  expressions use `IfExpression`, `Else`, and `BranchMerge`; while syntax is
  parsed by `AstBuilder::parseLeadingExpression()` and later lowered by
  `AstBuilder::postProcessWhileLoop()` into a generated `_body` function,
  `While`, `Call`, and `GoTo`.
- Implement traversal, numbering, instruction count, serialization, internal
  links, and completion behavior in
  [`expression.cpp`](../../../src/compiler/expression.cpp).
- In [`GraphLinker`](../../../src/compiler/graphLinker.cpp), capture state before
  divergent paths and merge it only at a stable convergence instruction.
- Route dependencies leaving nested control flow through its stable completion,
  rather than whichever syntactic statement happens to be last.
- Ensure identifiers declared inside the construct expire at the correct scope
  boundary.

## Runtime checklist

- Add execution and skip behavior in
  [`Executor::execSingleInstruction()`](../../../src/interpreter/executor.cpp).
- Make the skipped path publish the same number of dependency signals as the
  executed path requires.
- Do not classify a nested asynchronous call as terminal merely because the
  `Call` opcode itself has run.
- For repeating constructs, keep reset and dependency publication under
  `dependencyStateMutex`. Publish loop-back work only after the iteration's
  terminal signals have been installed.
- Exercise zero, one, and multiple iterations and both sides of every branch.

## Concrete example: a loop inside `else`

The E2E case `ElseBlocksCanContainLoops` compiles a false `if`, enters the else
block, executes a generated loop body twice, then reads the loop-written value.
It covers branch snapshot restoration, skipped-then dependency propagation,
loop completion, and the post-merge read.

The focused verification run used:

```sh
build/Tests --gtest_filter='AstBuilder.buildsWhileStatements:linkGraph.linksElseBranchesFromPreBranchResourceState:E2E/E2EFixture.E2E/ElseBlocksCanContainLoops16'
```

The expected outcome is three passing tests. For a timing-sensitive change,
repeat the E2E case in a Release build instead of relying on this single run.

## Failure signatures

- a following read observes the pre-branch value: branch resource state was not
  merged or the selected write was skipped incorrectly;
- a program stalls: a skipped or empty path did not release a dependency;
- a following statement runs during a nested loop: completion was attached to
  a repeatable inner instruction instead of a stable outer signal;
- failures appear only with many workers: reset and dependency publication are
  racing, or reusable edges contain invocation-specific state.
