# Function declarations and lexical captures

[`FunctionExpression`](../../../src/compiler/expression.hpp) stores name, return type, parameters, body, loop-body metadata, saved compiler scope, resource-use summaries, and return sites. [`GraphLinker::enterFunction`](../../../src/compiler/graphLinker.cpp) first creates the function resource in the enclosing scope, then switches to a cloned lexical resource scope and creates parameter resources. The clone preserves the lexical binding structure and function references, but creates fresh `Resource` objects with reset access state so analysis mutations do not leak back to the caller's working state.

On exit, [`GraphLinker::exitFunction`](../../../src/compiler/graphLinker.cpp) derives `firstUses`, `firstWrites`, `lastUses`, and `lastWrites`. Parameters participate in first-use mapping (call arguments initialize them) but are excluded from outward last-use/write effects. Locals and the function's own resource are likewise excluded from capture summaries.

At runtime, the `Function` instruction builds a [`Function`](../../../src/interpreter/function.cpp), allocates it in the current `Scope<Value>`, and skips its body template. The constructor slices the following block into a reusable `Subprogram`; calls clone it.

## Evidence

This 16-worker regression passed in the current run:

```parallel
int outer(int captured) {
  int inner(int n) {
    if (n == 1) return captured;
    else return inner(n - 1);
  }
  return inner(3);
}
print outer(7);
```

[`NestedRecursiveFunctionsRetainTheirLexicalScope`](../../../tests/e2e/tests.cpp) observed `7`. Parameter shadowing is covered by [`FunctionParametersShadowVariablesWhenCalled`](../../../tests/e2e/tests.cpp).

The key invariant is that declaration-time lexical visibility is preserved, while every invocation receives fresh parameter storage. Capturing the caller's current dynamic scope or sharing parameter scopes causes cross-call contamination.

## Change workflow

A contributor extending captures must update both phases: `GraphLinker` must
record the captured resource in the function summaries, and the executor must
retain the corresponding enclosing `Scope<Value>` when cloning the call body.
The compiler tests should assert the exact `firstUses` or `lastUses` entry, and
an E2E test should call the same declaration concurrently with different
arguments. `NestedRecursiveFunctionsRetainTheirLexicalScope` is the concrete
model for a recursive capture regression.
