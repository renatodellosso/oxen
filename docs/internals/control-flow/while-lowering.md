# While-loop lowering

The parser initially builds a `While` plus its following block. [`AstBuilder::postProcessWhileLoop`](../../../src/compiler/astBuilder.cpp#L449) rewrites that pair into an outer block containing:

1. a synthetic `FunctionExpression("_body", "void", ..., generatedLoopBody=true)` holding the original body;
2. the condition expression and `While` instruction;
3. an inner block containing a call to `_body` and a backward `GoTo`.

The jump distance spans back to condition evaluation. The outer block sets `completionExpression` to `While`, giving enclosing branches/calls one stable end signal. [`FunctionExpression::toByteCode`](../../../src/compiler/expression.cpp#L425) serializes the `generatedLoopBody` flag; [`Function::Function`](../../../src/interpreter/function.cpp#L5) restores it at runtime.

The linker treats `GoTo` specially in [`GraphLinker::processExpression`](../../../src/compiler/graphLinker.cpp#L338): it depends on work between the target and jump, redirects repeatable completions to the condition, skips nested-loop ranges, and remaps generated body calls. Pre-loop dependencies entering the call are also copied to condition evaluation so a false first condition cannot release the post-loop continuation prematurely.

## Observed lowering

For `while (value < 2) value = value + 1`, `build/CLI ... -c -d` produced this recognizable sequence:

```text
14 Function void _body true
24 While
28 Call
29 GoTo -8
```

The same example prints `branch-loop=2` with 16 workers. [`buildsWhileStatements`](../../../tests/compiler/astBuilder.cpp#L371) checks the AST rewrite, and [`whileConditionDependsOnPreLoopWritesUsedByItsBody`](../../../tests/compiler/compiler.cpp#L96) checks the false-first-iteration ordering edge.

Do not lower a loop without marking its body function and stable completion; generic functions and repeatable loop bodies have different runtime completion behavior.
