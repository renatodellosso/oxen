# Nested calls and completion

Nested calls appear in argument expressions and inside callee bodies. [`CallExpression::addArgument`](../../../src/compiler/expression.cpp#L616) preserves the expression tree, while serialization maps each argument value to the parameter's first uses. Runtime call setup gates the cloned root block on every argument, so an inner result cannot arrive after an outer body starts reading its parameter.

Completion discovery must also look through nested call structure. [`getCompletionInstructionIds`](../../../src/interpreter/executor.cpp#L100) does not mark a `Call` terminal when it has an active dependent inside the same cloned body; the continuation after that call is the terminal. Completion propagation from the nested invocation eventually reaches the outer barrier.

Concrete examples are [`CallsCanBeNestedAsArguments`](../../../tests/e2e/tests.cpp#L578), where `twice(increment(2))` prints `6`, and [`NestedRecursiveFunctionsRetainTheirLexicalScope`](../../../tests/e2e/tests.cpp#L628), which prints `7`. The focused completion-barrier tests pass.

Failure modes include an outer call reading an uninitialized parameter, terminal discovery releasing at the nested `Call` rather than its continuation, and counting both nested and propagated completion signals.
