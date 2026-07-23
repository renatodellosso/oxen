# Completion semantics

“Complete” is a graph property, not the same thing as “an instruction returned a value.” An ordinary [`Expression::getCompletionExpressions`](../../../src/compiler/expression.cpp#L99) returns itself. A [`BlockExpression`](../../../src/compiler/expression.hpp#L153) returns the union of its top-level statements' completions, unless `completionExpression` names one stable signal. An [`IfExpression`](../../../src/compiler/expression.cpp#L347) always completes at its synthetic `BranchMerge`.

This distinction matters for repeated control flow. [`AstBuilder::postProcessWhileLoop`](../../../src/compiler/astBuilder.cpp#L449) marks the lowered loop block's completion as its `While`, because body and `GoTo` instructions execute repeatedly. Linking a continuation to either would over-fulfil a fixed dependency count. Runtime call completion is a third layer: [`getCompletionInstructionIds`](../../../src/interpreter/executor.cpp#L99) finds terminal instructions in a cloned function body, and [`getStableCompletionInstructionIds`](../../../src/interpreter/executor.cpp#L131) collapses repeating loop terminals to the `While`.

Invariant: every logical construct exposes signals that fire exactly once for that execution. A value-producing call may release its expression consumer through `ReturnInvocation` before the invocation's side effects finish; resource and sequencing dependents use `CallCompletion` barriers instead.

## Concrete evidence

The current focused run was:

```text
build/Tests --gtest_filter='ExpressionCompletion.*:startExecution.waitsForTerminalCallSideEffectsAfterDependencyRemapping'
[==========] Running 6 tests from 2 test suites.
[  PASSED  ] 6 tests.
```

The five tests in [`tests/compiler/expression.cpp`](../../../tests/compiler/expression.cpp#L69) cover ordinary, empty-block, aggregate-block, branch-merge, and explicit loop completion. The executor test in [`tests/interpreter/executor.cpp`](../../../tests/interpreter/executor.cpp#L519) runs eight calls with 16 workers and requires their terminal `Print` side effects to finish before the next write.

Failure signs are a continuation running twice, a branch merge releasing early, or a later write overtaking a callee side effect. When adding control flow, define its stable completion before adding dependencies.
