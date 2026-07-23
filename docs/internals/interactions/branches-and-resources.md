# Branches and resource dependencies

Branches combine the structural join in [`IfExpression::linkInternally`](../../../src/compiler/expression.cpp#L352) with the resource-state join in [`GraphLinker::finalizeBranchMerge`](../../../src/compiler/graphLinker.cpp#L293). The former says when both taken/skipped paths are accounted for; the latter says which expression represents a resource after the choice.

Example:

```parallel
int a = 0;
if (true) a = 1; else a = 2;
print a;
```

Both branches are linked from the snapshot containing `a = 0`. Because either branch writes `a`, the merge becomes `a.lastWrittenBy`; the final read waits on it. The untaken block is skipped but publishes its non-value merge edges.

Current evidence: `build/Tests --gtest_filter='linkGraph.linksElseBranchesFromPreBranchResourceState:E2E/E2EFixture.E2E/ElsesSetVariablesFromThenBranch16'` passed both selected tests in the current build; the E2E case is defined at [`tests/e2e/tests.cpp`](../../../tests/e2e/tests.cpp#L179). The direct false/else example printed `branch-loop=2` with 16 workers.

Invariant: an else access must never depend on a then write, and a post-branch
access must never bypass `BranchMerge`. When adding resource-like effects,
update snapshot capture, branch-use marking, and merge finalization together.

## Contributor workflow

The smallest useful linker regression records the initial write, one write per branch, the merge, and the final read, then asserts every dependency explicitly. A second case should leave one branch read-only so that merge behavior is not accidentally applied to untouched resources. The E2E layer should execute both condition values with 16 workers and verify the final value rather than instruction timing.
