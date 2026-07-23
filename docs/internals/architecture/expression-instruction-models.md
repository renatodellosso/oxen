# Expression and instruction object models

The compiler model is an owning tree plus non-owning graph edges. The runtime model is a flat, owned instruction buffer plus pointers within that buffer.

## Compiler expressions

[`Expression`](../../../src/compiler/expression.hpp) stores an `InstructionType`, source line, mutable bytecode `id`, `dependencies`, `dependents`, and optional `dependentRedirect`. Tree ownership comes from derived classes:

- `RootExpression` owns a `Token`.
- `UnaryExpression` owns `root`; `BinaryExpression` owns `left` and `right`.
- `BlockExpression` owns ordered statement roots and may name a synthetic `completionExpression`.
- `IfExpression` owns condition, branch blocks, `Else`, and `BranchMerge` nodes.
- `FunctionExpression` owns its body and resource-use summaries.
- `CallExpression` is a block whose first child is a `UnaryCallExpression`; later children are generated parameter-initializing `Set` expressions.

`shared_ptr` owns nested expressions. `reference_wrapper` and raw `Expression *` never own nodes: they form dependency edges, summaries, redirects, and identity keys. Consequently, moving a `shared_ptr` is safe, but replacing or destroying its target while graph references remain is not.

[`addDependency()`](../../../src/compiler/expression.cpp) updates both directions exactly once. `Expression::dependencies` is a vector because only the count and incoming relationship matter at serialization. `dependents` is an `unordered_set<ExprDependent>` deduplicated by destination identity plus optional argument index. An argument index means the runtime result is written to that destination's `depArgs[index]`.

`dependentRedirect` changes where later resource-order dependencies terminate. It is used for terminal side effects, calls, and loop completion; it does not transfer ownership. `BlockExpression::completionExpression` similarly declares a stable completion node for a synthetic block.

## Runtime instructions

[`Instruction`](../../../src/instruction.hpp) is the flat counterpart. `bytecodeArgs` are immutable operands encoded on its line; `depArgs` are results supplied by predecessor instructions. `depCount`/`depsFulfilled`, `queued`, `skipped`, and `executed` are scheduler state. Each instruction also carries its runtime `Scope<Value>` and owning `Subprogram`.

[`InstrDependent`](../../../src/instruction.hpp) points to an `Instruction` and preserves the optional argument index. Its additional `disabled`, `ReturnInvocation`, and `CallCompletion` fields are invocation/runtime state and have no compiler equivalent. Reusable call-edge metadata must not become a storage location for mutable per-invocation state.

[`Subprogram`](../../../src/interpreter/subprogram.cpp) is responsible for pointer validity. The range constructor copies instructions and repairs internal dependent pointers; the shared-buffer constructor repairs pointers after taking ownership of an already-built vector. `clone()` uses the range constructor for each function invocation.

## Traversal contract

`getWithSubExpressions()`, `numberExpressions()`, `countInstructions()`, and `toByteCode()` must describe exactly the same order. For `BinaryExpression`, that order is left subtree, right subtree, binary node. For `BlockExpression`, it is block node followed by each statement tree. Violating this four-method contract produces wrong IDs, block sizes, or dependent pointers even if the AST prints correctly.

## Concrete observed example

I ran:

```sh
build/Tests --gtest_filter='compile.compilesBasicProgram:buildInstructions.buildsSingleInstruction:linkGraph.linksInternally'
```

All three tests passed. For `1 + 2`, the verified model is:

```text
Expression IDs: literal 0, literal 1, Add 2
Edges: 0 -> 2.arg[0], 1 -> 2.arg[1]
Bytecode: 0 2.0 1 1
          0 2.1 1 2
          2  6
Runtime: instruction 2 has depCount=2 and receives two depArgs
```

## Change hazards

- Adding a derived expression without overriding all relevant traversal methods desynchronizes IDs and lines.
- Copying an expression after linking copies raw/reference edges that still identify old nodes.
- Changing `ExprDependent` equality or hashing changes deduplication and bytecode dependent order.
- Holding `Instruction *` across vector reallocation is unsafe; construct the complete vector before resolving pointers, as `BytecodeParser` does.
- `executed` is deliberately not equivalent to current-iteration completion; loop reset semantics use other fields.
