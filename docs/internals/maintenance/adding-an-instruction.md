# Adding an instruction

Adding an instruction is a vertical change: the compiler must create it, the
bytecode must preserve it, and the executor must implement it. Treating only
one layer usually produces bytecode that compiles but either parses incorrectly
or reaches an unhandled executor case.

## The implementation path

1. Add the opcode to `InstructionType` in
   [`src/instruction.hpp`](../../../src/instruction.hpp) and its diagnostic name
   to `instructionTypeToString()` in
   [`src/instruction.cpp`](../../../src/instruction.cpp). The enum's integer
   value is serialized, so reordering existing entries changes the bytecode
   format.
2. If new syntax is required, add a `TokenType` in
   [`src/compiler/token.hpp`](../../../src/compiler/token.hpp), recognize it in
   `Tokenizer::parse()` or its helpers in
   [`tokenizer.cpp`](../../../src/compiler/tokenizer.cpp), and construct the
   corresponding expression in `AstBuilder::parseLeadingExpression()`,
   `AstBuilder::parseCompoundExpression()`, or a construct-specific helper in
   [`astBuilder.cpp`](../../../src/compiler/astBuilder.cpp).
3. Choose an expression shape from
   [`expression.hpp`](../../../src/compiler/expression.hpp). `RootExpression`,
   `UnaryExpression`, and `BinaryExpression` already define traversal,
   numbering, internal dependency linking, and serialization order. A new
   shape must implement all of those contracts itself.
4. Teach `GraphLinker::processExpression()` in
   [`graphLinker.cpp`](../../../src/compiler/graphLinker.cpp) about every
   resource read, write, side effect, or control-flow edge introduced by the
   instruction. Call `Expression::redirectResourceCompletionsTo()` when a read
   must remain live until a terminal side effect finishes.
5. Extend expression bytecode arguments if necessary. The common prefix is
   emitted by `Expression::toByteCode()`. `BytecodeParser::buildSingleInstruction()`
   in [`bytecodeParser.cpp`](../../../src/interpreter/bytecodeParser.cpp)
   already parses trailing values generically; structural arguments such as
   block sizes and function metadata are interpreted later by the runtime.
6. Add execution behavior to `Executor::execSingleInstruction()` in
   [`executor.cpp`](../../../src/interpreter/executor.cpp). Validate argument
   types before extracting a `std::variant` value, publish results through
   `updateDependency()`, and do not mutate reusable dependency edges with
   invocation-specific state.

## Invariants to check

- `getWithSubExpressions()`, `numberExpressions()`, `countInstructions()`, and
  `toByteCode()` must describe the same order and instruction count.
- Every internal operand edge uses the correct `argIndex`, because the index
  selects a slot in `Instruction::depArgs`.
- Every execution path eventually releases or deliberately skips its
  dependents.
- An instruction that can repeat in a loop must be safe after the executor
  resets its scheduling fields.
- Any new bytecode argument must survive strings, negative integers, and debug
  comment lines as applicable.

## Concrete model: `CompareNotEquals`

The existing inequality instruction demonstrates the shortest complete path.
`TokenType::NotEquals` is recognized by the tokenizer,
`AstBuilder::parseCompoundExpression()` maps it to
`InstructionType::CompareNotEquals`, `BinaryExpression::linkInternally()`
connects its two operands, and `Executor::execSingleInstruction()` shares the
equality comparison implementation before negating the result.

Run:

```sh
build/Tests --gtest_filter='Tokenizer.identifiesComparisonOperators:AstBuilder.buildsComparisonExpressions:startExecution.compareNotEqualsComparesPrimitives:E2E/E2EFixture.E2E/InequalityWorksWithBools16'
```

Expected result: four selected tests pass. This combination checks recognition,
AST construction, direct executor semantics, and the complete compiler/runtime
path with 16 workers.

## Minimum test set for a change

- tokenizer test for new spelling;
- AST test for expression shape and operands;
- graph-linker test for resource and control-flow edges;
- bytecode parser test if the serialized shape changes;
- executor unit test for success and invalid operands;
- E2E tests at all configured worker counts in
  [`tests/e2e/e2e.cpp`](../../../tests/e2e/e2e.cpp).
