# Unordered graph structures and deterministic bytecode

The compiler deliberately uses unordered containers for sets and summaries, but bytecode encodes some relationships by position. A change is safe only when positional values are derived from the exact emission order or when the data is explicitly ordered first.

## Where order matters

[`Expression::dependents`](../../../src/compiler/expression.hpp) is an `unordered_set<ExprDependent>`, and [`Expression::toByteCode()`](../../../src/compiler/expression.cpp) emits it in iteration order. This is semantically valid because each entry carries an absolute destination ID and optional argument index.

Call dependent remaps are different: their bytecode refers to an index within that emitted dependent list. `UnaryCallExpression::depRemaps` therefore stores destination `Expression *` identity. `UnaryCallExpression::toByteCode()` walks `dependents`, increments `dependentIndex`, and emits a remap only when that exact destination pointer has one. The base line and remap metadata consequently use one iteration order within the same serialization operation.

Function parameter remaps use numeric parameter index, which comes from the ordered `params` vector. Subprogram instruction references use final expression IDs. `GraphLinker::expressions` uses `std::map` to guarantee increasing-ID processing.

## Where order should not matter

`Scope` variable tables, resource snapshots, touched/written resource sets, and function last-use maps represent name-keyed facts. Their traversal may alter diagnostics or the order of independent serialized metadata, but it must not change graph meaning. If a format consumer begins to attach meaning to their order, introduce an explicit sort or ordered structure at that boundary.

## Concrete failure pattern

The unsafe pattern is:

```text
link time: remember dependent position 2 from unordered_set iteration A
emit time: iterate after mutation/rehash in order B
runtime: remap position 2, now a different dependent
```

The implemented pattern is:

```text
link time: depRemaps[dependent Expression*] = callee completion nodes
emit time: for dependent in actual emitted order, resolve its current index
```

I ran:

```sh
build/Tests --gtest_filter='ExprDependent.*:linkGraph.setsCallDepRemaps:E2E/E2EFixture.E2E/FunctionsCanBeCalledInsideLoops16:E2E/E2EFixture.E2E/CallsAreSequencedCorrectly16'
```

Observed: all selected tests passed, including the 16-worker loop/call and call-sequencing regressions. These exercise the identity-to-index boundary under the paths most sensitive to a wrong remap.

## Review checklist

- Does any stored integer mean “the nth item” in an unordered container? Replace it with stable identity until emission.
- Are related index and list fields emitted from the same ordered snapshot?
- Can inserting another dependent rehash the set between linking and serialization?
- Are final bytecode IDs resolved only after the second numbering pass?
- Does a test parse the bytecode and verify the remap destination, plus stress the affected runtime interaction?

Byte-for-byte reproducibility is a separate goal from semantic correctness. To guarantee it, define canonical sorting for every unordered emission. Sort complete entries with their associated metadata; sorting only dependent IDs while leaving remap indices untouched corrupts the relationship.
