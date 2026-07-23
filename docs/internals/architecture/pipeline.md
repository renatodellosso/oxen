# Compilation and execution pipeline

Parallel source crosses two explicit API boundaries: `compileToBytecode()` turns a source stream into text, and `executeBytecode()` turns that text into scheduled instructions. These boundaries are useful in tests and should remain usable without the CLI.

## Phase map

1. [`compile()`](../../../src/compiler/compiler.cpp) constructs a [`Tokenizer`](../../../src/compiler/tokenizer.hpp), calls `parse()`, and transfers its `vector<Token>` with `close()`.
2. [`AstBuilder::build()`](../../../src/compiler/astBuilder.cpp) consumes those tokens into owning `shared_ptr<Expression>` roots. Its private `postProcess()` pass also lowers loops and attaches function bodies.
3. `compile()` calls `Expression::numberExpressions()` on every root. [`GraphLinker`](../../../src/compiler/graphLinker.cpp) requires these provisional IDs because it builds an ordered `map<int, reference_wrapper<Expression>>` and uses ID ranges for scopes, branches, and loops.
4. `GraphLinker::linkGraph()` adds internal data-flow edges, resource-ordering edges, control-flow edges, function summaries, and deferred recursive-function links.
5. `compile()` numbers the tree again after linking. The current linker changes
   edges and call metadata rather than the expression tree, so the second pass
   normally reproduces the provisional IDs; the IDs used during serialization
   are nevertheless those from this final pass.
6. Every root emits text through `Expression::toByteCode()`. The output order is the same recursive order used by `numberExpressions()`.
7. [`BytecodeParser::buildInstructions()`](../../../src/interpreter/bytecodeParser.cpp) creates a contiguous `vector<Instruction>`, then resolves textual dependent IDs to pointers.
8. [`executeBytecode()`](../../../src/interpreter/interpreter.cpp) moves that vector into a [`Subprogram`](../../../src/interpreter/subprogram.cpp), repairs dependent pointers, and starts an [`Executor`](../../../src/interpreter/executor.cpp).

The CLI paths in [`executeCommand()`](../../../src/cli.cpp) preserve these boundaries: compile only writes a file; interpret only parses bytecode; the default compile-and-interpret mode uses an in-memory `istringstream`.

## Central invariants

- Expression identity is stable from AST construction through serialization. IDs are mutable positions; pointers identify nodes.
- Each serialized instruction occupies exactly one non-comment bytecode line. Its ID is that line's zero-based instruction position.
- An expression's `dependencies.size()` must equal the serialized dependency count, and every reverse `ExprDependent` must point to the final ID of its destination.
- Graph linking happens after provisional numbering and before final numbering/serialization.
- `Subprogram` owns the instruction buffer for as long as any `InstrDependent::instr` points into it.
- Compiler errors stop before bytecode is written. Parser and executor failures are translated separately by `executeBytecode()`.

## Concrete observed example

The compiler test `compile.compilesBasicProgram` sends `1 + 1` through `compile()` and checks this exact bytecode (`GetLiteral == 1`, `Add == 6`):

```text
0 2.0 1 1
0 2.1 1 1
2  6
```

The first two instructions publish values to dependency slots 0 and 1 of instruction 2; instruction 2 waits for both. The following command exercises every compiler boundary shown in the phase map:

```sh
build/Tests --gtest_filter='Tokenizer.*:compile.*:AstBuilder.*:linkGraph.*:buildInstructions.*'
```

Observed: 86 tests ran and all 86 passed, including
`compile.compilesBasicProgram` and
`compile.whileConditionDependsOnPreLoopWritesUsedByItsBody`.

## Change hazards

When adding a syntax form, check every boundary: token
classification, AST ownership, recursive traversal, provisional numbering,
linking, final numbering, bytecode arguments, parser interpretation, executor
dispatch, and focused plus end-to-end tests. Never use provisional IDs as
permanent identity. Never insert emitted instructions without updating
`getWithSubExpressions()`, `numberExpressions()`, `countInstructions()`, and
`toByteCode()` consistently.
