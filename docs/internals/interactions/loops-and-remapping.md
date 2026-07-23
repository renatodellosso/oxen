# Loops and dependency remapping

Loop reset operates on static instruction ranges, while calls create fresh subprograms. The bridge is `UnaryCallExpression::depRemaps` plus stable completion normalization. For a `GoTo`, the call-aware linker [`addDependency`](../../../src/compiler/graphLinker.cpp#L40) maps the jump to all callee `lastUses`, following `dependentRedirect` chains. For resource-specific consumers it maps only the corresponding last uses.

At runtime the `Call` copies these static descriptions into invocation-local edges and completion barriers. [`getStableCompletionInstructionIds`](../../../src/interpreter/executor.cpp#L132) removes completion IDs inside a repeated loop region and inserts the `While`; [`releasesGoTo`](../../../src/interpreter/executor.cpp#L166) unwraps nested barriers so loop-back publication remains last.

Debug compilation of the direct loop showed a serialized call with a dependent remap followed by `GoTo -8`; execution with 16 threads printed `loop-call=0`, `1`, `2`. The focused [`waitsForCallsInsideLoopIterations`](../../../tests/interpreter/executor.cpp#L551) test passed and additionally asserts reusable call dependents are not left disabled after execution.

Invariant: remap descriptions are reusable and immutable; invocation completion state is fresh. Persistent mutation produces failures only on later iterations or concurrent calls.
