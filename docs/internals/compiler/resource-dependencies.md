# Resource dependency analysis

The compiler orders accesses to names without serializing unrelated expressions. [`Resource`](../../../src/compiler/resource.hpp) is the analysis record for one lexical name, not a runtime value. It stores `lastWrittenBy`, the accesses since that write in `currAccesses`, and optional function metadata.

## The access algorithm

[`GraphLinker::useResource()`](../../../src/compiler/graphLinker.cpp) implements three ordering rules:

- Read after write: a read depends on `lastWrittenBy` when it exists.
- Write after read: a write depends on every expression in `currAccesses`.
- Write after write: the previous write is itself in `currAccesses`, so the same loop adds that edge too.

After a read, the read is appended to `currAccesses`. After a write, `lastWrittenBy` becomes the write, the prior access set is cleared, and the new write starts the next access set. `createResource(Expression&)` initializes a declaration as both the last write and first current access.

`processExpression()` recognizes `Declare`, identifier reads (`GetIdentifier` and `ReferenceIdentifier`), and `Set`. A `Set` also has ordinary internal dependencies on its left and right operands. `deduplicateDependenciesForSet()` removes the unindexed duplicate edge created when the left identifier is both a write target and a binary operand.

Built-in names `int`, `bool`, and `string` are resources created by the `GraphLinker` constructor. A declaration looks up its type as an identifier while allocating its variable name.

## Scope and control-flow interaction

Resources live in `Scope<Resource>`, so a nested declaration shadows rather than mutates the enclosing analysis record. Blocks push a child scope and `scopeLifetimes` determines when traversal restores the enclosing scope.

Branches cannot simply process `then` followed by `else`: that would falsely order the else branch after the then branch. `captureResourceSnapshot()`, `enterElseBranch()`, and `finalizeBranchMerge()` restore the pre-branch state and join touched resources at `BranchMerge`. Functions record `firstUses`, `firstWrites`, `lastUses`, and `lastWrites`. Calls use `lastUses`/`lastWrites` to model callee effects in the caller's resource graph, while call serialization uses parameter `firstUses` to route argument values inside the cloned body.

`Expression::redirectResourceCompletionsTo()` handles a subtler ordering rule. A read consumed by `Print` or a function invocation redirects a later write to the terminal side effect/call, so mutation cannot overtake the consumer after the identifier value has merely been fetched.

## Concrete observed example

For:

```text
int var;
var;
var = 1;
```

the declaration is the first write. The standalone read depends on it. Before the final `Set` is processed, its `ReferenceIdentifier` target is also registered as a read. The `Set` waits for the declaration and both reads (with its duplicate unindexed target edge removed), then becomes `lastWrittenBy` and the only entry in the new `currAccesses` set.

The following focused command verifies the default resources, read/write
ordering, and redirected print completion:

```sh
build/Tests --gtest_filter='constructor.createsDefaultResources:linkGraph.createsResources:linkGraph.readsResources:linkGraph.writesResources:linkGraph.writesWaitForPriorPrintSideEffects'
```

Observed: all 5 passed. The tests inspect exact expression addresses in both edge directions and verify the resource's final state.

## Invariants and hazards

- Every resource edge is added in both directions through `addDependency()`.
- A write must wait for all accesses since the prior write, not only the last read.
- A branch must begin from the pre-branch snapshot and join at the merge.
- Resource identity is lexical: compare the `shared_ptr<Resource>` returned by scopes when deciding whether a use belongs to a function or a shadow.
- Calls require a resource name for ordinary remapping; only loop `GoTo`
  handling intentionally remaps across all callee last uses.
- A side-effecting consumer change explicitly determines whether read
  completion redirects through it.

A contributor extending resource analysis first identifies whether the new
expression reads, writes, or terminally consumes a lexical resource. The
implementation then updates `GraphLinker::processExpression()` and, when
needed, `Expression::redirectResourceCompletionsTo()`. Tests in
[`tests/compiler/graphLinker.cpp`](../../../tests/compiler/graphLinker.cpp)
verify both `dependencies` and reverse `dependents`; a concurrent E2E or
executor regression is also required when the edge controls side-effect order.
