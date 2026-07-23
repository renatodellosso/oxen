# Calls and resource side effects

The compiler summarizes captured resource reads/writes in [`FunctionExpression::lastUses`](../../../src/compiler/expression.hpp#L208) and `lastWrites`. A caller's [`GraphLinker::useResource`](../../../src/compiler/graphLinker.cpp#L176) treats the `Call` as a read or write of those resources, then call-aware dependency remapping routes later resource users to matching instructions inside the cloned body.

`Print` needs special treatment: [`Expression::redirectResourceCompletionsTo`](../../../src/compiler/expression.cpp#L81) redirects identifier-read completion through the terminal side effect. Otherwise the next writer could begin after the read but before output was emitted. Empty call remaps are expanded at runtime with terminal completion IDs and collected by `CallCompletion`.

The focused [`waitsForTerminalCallSideEffectsAfterDependencyRemapping`](../../../tests/interpreter/executor.cpp#L511) test uses 16 workers. Its program alternates `printValue()` with `value = value + 1` eight times and requires ordered output `0` through `7`.

Failure signature: outputs show a newer resource value than the call should have observed, or a later write completes before a callee's print.
