# Recursion, deferred linking, and scope

Recursive compilation needs two stable snapshots: function usage from the completed first pass and the function's original lexical scope. [`GraphLinker::linkDeferred`](../../../src/compiler/graphLinker.cpp#L748) preserves former summaries in `deferredFunctionUsage`; [`GraphLinker::enterFunction`](../../../src/compiler/graphLinker.cpp#L584) clones the saved `FunctionExpression::scope` before rebuilding first/last uses.

At runtime every recursive `Call` clones the body and gives its root block a child value scope. Parameters are therefore invocation-local, while captured values resolve through enclosing lexical scopes. A nested recursive function captures the outer invocation's parameter scope, not a global or sibling call's scope.

Current evidence: the focused linker tests for recursive resource reuse and captured-write dependencies both passed. The 16-worker runs of [`RecursiveFibonacciCallsCanBeLinked`](../../../tests/e2e/tests.cpp#L620) and [`NestedRecursiveFunctionsRetainTheirLexicalScope`](../../../tests/e2e/tests.cpp#L628) passed, observing `2` and `7` respectively. [`waitsForLeadingSideEffectsBeforeRecursiveCalls`](../../../tests/interpreter/executor.cpp#L602) produced an ordered logical countdown from 12 to 0.

Typical failures are redeclaring the recursive function during pass two, clearing summaries before recursive calls can consult them, or cloning the wrong enclosing scope.
