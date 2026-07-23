# AST parsing and expression extension

[`AstBuilder`](../../../src/compiler/astBuilder.hpp) is a hand-written, cursor-based parser. It consumes the tokenizer's vector and builds top-level `shared_ptr<Expression>` roots; parsing methods temporarily use `unique_ptr` to make ownership transfer explicit.

## Parsing flow

`build()` repeatedly calls `parseExpression()` and then invokes `postProcess()`. `parseExpression()` catches `runtime_error`, records a `SyntaxError`, and returns no expression. `next()` updates the builder's current line from the consumed token.

`extendExpression()` first obtains a leading expression and, unless the expression auto-terminates, calls `parseCompoundExpression()`. Leading forms include literals/identifiers, blocks, return, if, while, and print. Compound forms include binary operators, declarations, assignment, and calls.

There is no precedence table. `parseCompoundExpression()` consumes an operator and parses the entire remaining right operand through `extendExpression(nullptr, endOn)`. Operators therefore group from the right and share one precedence mechanism. For example, `2 * 3 + 4` becomes `Multiply(2, Add(3, 4))`, while `8 / 4 / 2` becomes `Divide(8, Divide(4, 2))`.

Declarations are recognized when an identifier expression is followed by another identifier. The first is treated as a type, and the name is represented as `GetLiteral` so runtime declaration can allocate it. For assignment to an existing name, `=` changes the preceding `GetIdentifier` into `ReferenceIdentifier`; an initializer instead makes the whole preceding `Declare` the left side of `Set`. Calls are represented as a `CallExpression` block, and `addArgument()` wraps each source argument in a generated declaration/set; `setFunction()` fills in parameter type/name after linking resolves the callee.

`parseStatementBlock()` normalizes a single `if` or `else` body into `BlockExpression`, so `if (true) print 1;` and a braced body share downstream structure. `else if` works because the else body may itself parse an `IfExpression`. Source `while` and function bodies are instead parsed as the next top-level/block expression and attached during post-processing.

## Post-processing

`postProcess()` recursively visits blocks and `IfExpression` branches. For the provisional `While` and `Function` forms, it takes the following expression as the body, first wrapping a non-block expression in `BlockExpression`. It then attaches a function body or asks `postProcessWhileLoop()` to lower a while/body pair into a synthetic outer block containing:

1. generated function `_body`,
2. the original body (later attached to that function),
3. copied `While` condition instruction,
4. inner block containing `_body()` and backward `GoTo`.

It marks the function `generatedLoopBody` and sets the outer block's `completionExpression` to the stable while instruction.

## Concrete observed examples

I ran:

```sh
build/Tests --gtest_filter='AstBuilder.*:E2E/E2EFixture.E2E/MixedArithmeticOperatorsAreGroupedFromTheRight1:E2E/E2EFixture.E2E/DivisionChainsAreGroupedFromTheRight1'
```

Observed: all 26 AST tests and both E2E cases passed. The E2E outputs were `14` for `2 * 3 + 4`, `7` for `1 + 2 * 3`, and `4` for `8 / 4 / 2`, confirming current right grouping rather than conventional precedence/left associativity.

## Invariants and hazards

- End tokens are owned by the surrounding parse method; consuming one too early shifts the cursor.
- Auto-ending control-flow/function expressions must not be extended as binary operands accidentally.
- Every synthetic node introduced in post-processing needs correct line data, traversal/count behavior, and `postprocessed` handling.
- Changing precedence is a language-visible and graph-shape change; update AST, compiler-bytecode, and E2E expectations together.
- A function declaration is incomplete until post-processing attaches its body; do not run numbering/linking before `build()` finishes.
