# Deferred and recursive function linking

A call cannot be fully linked until its callee has complete resource-use summaries. During [`GraphLinker::processExpression`](../../../src/compiler/graphLinker.cpp), encountering a call to a function whose `finishedLinking` is false marks the immediately enclosing function `deferred` and records it in `deferredFunctionLinkings`.

[`GraphLinker::linkDeferred`](../../../src/compiler/graphLinker.cpp) then performs a second pass over only deferred functions. Before clearing summaries, it snapshots `lastUses` and `lastWrites` in `deferredFunctionUsage`; recursive calls encountered during rebuilding consult that completed first-pass snapshot. It clears branch contexts because those snapshots point into the old working scopes, reconstructs expression ordering, and relinks with clean clones of each saved lexical environment. [`enterFunction`](../../../src/compiler/graphLinker.cpp) deliberately does not redeclare the outer function resource during this pass.

Invariant: recursive calls see a stable previous summary until the replacement summary is complete. Deferred relinking must preserve the original lexical environment but discard transient access state from pass one.

## Evidence

The focused run passed [`linksRecursiveFunctionsWithoutRedeclaringTheirResource`](../../../tests/compiler/graphLinker.cpp) and [`recursiveCallsRetainCapturedResourceDependencies`](../../../tests/compiler/graphLinker.cpp). The former asserts exactly one `recursive` function resource; the latter asserts a recursive call depends on the preceding captured-resource write.

The 16-worker E2E [`RecursiveFibonacciCallsCanBeLinked`](../../../tests/e2e/tests.cpp) also passed and printed `2` for `fibonacci(3)`. Typical failures are duplicate function resources, empty recursive usage summaries, or nested recursive functions losing captured names.
