# Call compilation and dependency remapping

[`CallExpression::addArgument`](../../../src/compiler/expression.cpp) wraps each source argument in a synthetic parameter declaration/set expression. Once lookup succeeds, [`CallExpression::setFunction`](../../../src/compiler/expression.cpp) binds the `FunctionExpression` and fills those placeholders with the parameter names and types. [`CallExpression::linkInternally`](../../../src/compiler/expression.cpp) links argument initialization and the actual unary `Call`.

Resource ordering needs more than a direct edge to the call. The linker's call-aware [`addDependency`](../../../src/compiler/graphLinker.cpp) records `UnaryCallExpression::depRemaps`: a dependent expression identity maps to the callee's relevant `lastUses`. For a resource consumed in a call argument, an empty mapping means “wait for this whole invocation”; for loop `GoTo`, mappings cover all last uses. Identity (`Expression *`) is retained because `dependents` is unordered.

[`UnaryCallExpression::toByteCode`](../../../src/compiler/expression.cpp) resolves identities to emitted dependent indices only during serialization. Its payload contains dependent remaps, argument-value-to-first-use remaps, argument declaration offsets, and return instruction IDs. Reordering that format requires matching changes in the executor parse helpers near [`parseDependencyRemappings`](../../../src/interpreter/executor.cpp).

## Concrete bytecode evidence

The following source provides a minimal example:

```parallel
int identity(int value) {
  return value;
}
print identity(5);
```

Save the source as `/tmp/call.ox` and compile annotated bytecode with:

```sh
build/CLI -t /tmp/call.ox -c -o /tmp/call.bytecode -d
```

One emitted call line from the current build was:

```text
// (...)Call((...)GetIdentifier(identity))
1 11,9 21 0 1 5 1 1 1 3 1 2
```

Instruction IDs and dependent lists can change as lowering changes. The suffix
after instruction type `21` is the serialized remapping payload described
above. The focused unit run passed `linkGraph.setsCallDepRemaps`, while
[`waitsForTerminalCallSideEffectsAfterDependencyRemapping`](../../../tests/interpreter/executor.cpp)
passed with 16 workers.

Never store an unordered-set position during linking, and never mutate reusable
`InstrDependent` edges with invocation state. When changing the payload, update
`UnaryCallExpression::toByteCode()`, the parse helpers
in `executor.cpp`, and the graph-linker and executor tests named above in the
same change.
