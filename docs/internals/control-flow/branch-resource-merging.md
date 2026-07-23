# Branch resource snapshots and merging

Dependency analysis is linear, while branch execution is not. [`GraphLinker::registerIfExpression`](../../../src/compiler/graphLinker.cpp#L255) therefore captures a `ResourceState` snapshot before the then branch: each resource's shared object, `lastWrittenBy`, and `currAccesses`. [`markBranchResourceUse`](../../../src/compiler/graphLinker.cpp#L239) records resources touched or written by either branch.

Before linking `else`, [`enterElseBranch`](../../../src/compiler/graphLinker.cpp#L285) restores the pre-branch snapshot. This prevents an else read from depending on a write in the mutually exclusive then path. At `BranchMerge`, [`finalizeBranchMerge`](../../../src/compiler/graphLinker.cpp#L293) adds snapshot dependencies to the merge, sets `lastWrittenBy` to the merge for resources written on a path, and replaces current accesses with the merge. Consequently downstream users wait for branch choice and completion without knowing which path ran.

Invariant: each branch begins from the same incoming resource state; the merge becomes the post-branch state for any resource written in the branch. Nested contexts are all marked by expression-ID ranges, so an inner access also informs its enclosing branch.

## Evidence

[`linksElseBranchesFromPreBranchResourceState`](../../../tests/compiler/graphLinker.cpp#L429) constructs `a = 0; if (...) a = 1; else a = 2; print a;`. The focused run passed and asserts that the else reference depends on the initial set, not the then set; the merge depends on the initial set; and the later read depends on the merge.

The direct 16-thread CLI example likewise observed `branch-loop=2` after taking the else path. If snapshot restoration is removed, impossible cross-branch edges can deadlock or make an untaken write appear to precede the taken branch.
