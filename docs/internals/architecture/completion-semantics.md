# Completion semantics

“Complete” is a graph property, not the same thing as “an instruction returned a value.” An ordinary [`Expression::getCompletionExpressions`](../../../src/compiler/expression.cpp) returns itself. A [`BlockExpression`](../../../src/compiler/expression.hpp) returns the union of its top-level statements' completions, unless `completionExpression` names one stable signal. An [`IfExpression`](../../../src/compiler/expression.cpp) always completes at its synthetic `BranchMerge`.

This distinction matters for repeated control flow. [`AstBuilder::postProcessWhileLoop`](../../../src/compiler/astBuilder.cpp) marks the lowered loop block's completion as its `While`, because body and `GoTo` instructions execute repeatedly. Linking a continuation to either would overfulfill a fixed dependency count. Runtime call completion is a third layer: [`getCompletionInstructionIds`](../../../src/interpreter/executor.cpp) finds terminal instructions in a cloned function body, and [`getStableCompletionInstructionIds`](../../../src/interpreter/executor.cpp) collapses repeating loop terminals to the `While`.

Invariant: every logical construct exposes signals that fire exactly once for that execution. A value-producing call may release its expression consumer through `ReturnInvocation` before the invocation's side effects finish; resource and sequencing dependents use `CallCompletion` barriers instead.

For example, the lowered form of `while (false) print 1; print 2;` exposes the
`While` instruction as the loop block's stable completion. The following
`Print 2` can therefore receive one completion signal from the loop construct;
it does not depend on the repeating loop-body `Print` or `GoTo`. For a function
call whose return value feeds an expression while its body also prints, the
return consumer is released through `ReturnInvocation`, while a following
resource write waits for the `CallCompletion` barrier to collect the terminal
print signal.

## Concrete evidence

The current focused run was:

```text
build/Tests --gtest_filter='ExpressionCompletion.*:startExecution.waitsForTerminalCallSideEffectsAfterDependencyRemapping'
[==========] Running 6 tests from 2 test suites.
[  PASSED  ] 6 tests.
```

The five tests in [`tests/compiler/expression.cpp`](../../../tests/compiler/expression.cpp) cover ordinary, empty-block, aggregate-block, branch-merge, and explicit loop completion. The executor test in [`tests/interpreter/executor.cpp`](../../../tests/interpreter/executor.cpp) runs eight calls with 16 workers and requires their terminal `Print` side effects to finish before the next write.

Failure signs include a continuation running twice, a branch merge releasing
early, or a later write overtaking a callee side effect. When adding control
flow, define its stable completion before adding dependencies, add an
`ExpressionCompletion` graph test in
[`tests/compiler/expression.cpp`](../../../tests/compiler/expression.cpp), and
add an executor regression for repeated or concurrent behavior. The executor
test uses enough workers and invocations to exercise overlapping calls, and a
Release-mode stress run checks that one passing schedule is not mistaken for a
stable concurrency result.
