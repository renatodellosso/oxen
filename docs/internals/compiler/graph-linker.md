# GraphLinker traversal and state

[`GraphLinker`](../../../src/compiler/graphLinker.hpp) converts the AST's local tree edges into a dependency graph that expresses data flow, resource ordering, scope entry/exit, branches, calls, and loops.

## Construction and ordered traversal

The constructor flattens each root's `getWithSubExpressions()` into `expressions`, an ordered `map<int, reference_wrapper<Expression>>`. The AST must therefore be numbered before linker construction. `linkGraph()` iterates this map and calls `linkIteration()` for each node.

`linkIteration()` performs operations in a significant order:

1. Enter a function before processing its declaration node.
2. Update lexical scope lifetimes.
3. Run type-specific `processExpression()` logic.
4. Call the node's `linkInternally()`.
5. Decrement function instruction counts and exit completed functions.

Changing this order can cause resources to resolve in the wrong scope or internal edges to be missing from resource deduplication.

## State machines

`scope` and `scopeLifetimes` track compiler lexical scopes. Encountering a `Block` pushes a child `Scope<Resource>` for its flattened instruction count excluding the block instruction. The block adds a dependency to eligible immediate work in its flattened subtree (root expressions, nested blocks, and functions), while deliberately skipping deeper nodes whose internal edges already provide the gate.

`function`, `funcStack`, `funcExprsRemaining`, and `savedScopes` track nested functions. `enterFunction()` creates the function resource in the outer scope, clones its lexical resource environment, creates parameter resources, and saves that scope on `FunctionExpression`. `exitFunction()` derives first/last read/write summaries and restores the caller scope.

`branchContexts` stores a resource snapshot and ID ranges for each `IfExpression`. `Else` restores the snapshot; `BranchMerge` joins touched resources. The parallel lookup maps keyed by else/merge IDs avoid scanning by structural position.

`deferredFunctionLinkings` and `deferredFunctionUsage` support recursion. `linkDeferred()` rebuilds an ordered map containing only deferred function subtrees and re-links them against clean cloned lexical scopes while retaining completed first-pass summaries for recursive calls.

## Special expression processing

- `If`/`While`: the following block depends on the control instruction.
- `Block`: push scope, gate direct children, and resolve call targets.
- `Print`: redirect resource completion through the side effect.
- `GoTo`: wait for work in the iteration range, skip nested-loop ranges, propagate pre-loop call dependencies to the condition, and redirect loop-body dependents to the condition.
- Calls: convert arguments to parameter declarations, then apply callee usage summaries to caller resources.

`processExpression()` catches `runtime_error` from its type-specific graph construction and records it as `SyntaxError` through `syntaxError()`. Operations outside that function, such as `enterFunction()` and `linkInternally()`, are not covered by that catch.

## Concrete observed example

The regression source used by `compile.whileConditionDependsOnPreLoopWritesUsedByItsBody` is:

```text
int i = 0; while (false) i = i + 1; print i;
```

The test compiles and parses the bytecode, finds the initial `Set` and `While`, identifies the condition instruction through its `.arg[0]` dependent edge, and asserts that the initial set also points to that condition. I ran:

```sh
build/Tests --gtest_filter='linkGraph.*:compile.whileConditionDependsOnPreLoopWritesUsedByItsBody'
```

Observed: 26 tests ran and all passed (25 linker tests plus the compiler regression).

## Before/after invariants

Before `linkGraph()`: every expression must have a unique provisional ID; tree ownership and traversal counts must agree; no scope-dependent summary should be trusted. After a successful link: internal and external edges are bidirectional, functions are marked `finishedLinking`, deferred functions have replacement summaries, branch resources point at their merge where required, and collected processing errors are available through `getErrors()`.

Hazards include using unordered positions as identities, skipping scope-lifetime accounting when adding emitted nodes, changing loop lowering without revisiting ID ranges, and treating source-order traversal as execution order. Tests should inspect specific graph edges as well as end-to-end output.
