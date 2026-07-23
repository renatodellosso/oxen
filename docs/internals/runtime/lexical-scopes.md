# Lexical scopes in compiler and runtime

[`Scope<T>`](../../../src/scope.hpp) is shared by two phases with different payloads: `Scope<Resource>` tracks compiler dependency state, while `Scope<Value>` stores runtime variables.

## Lookup and allocation

Each scope owns an `unordered_map<string, shared_ptr<T>>` and an optional enclosing scope. `alloc()` always writes into the current map and returns the allocated shared object. `get()` checks the current scope first, then recursively checks `enclosing`; this implements shadowing. `contains()` follows the same chain. `getKeys()` returns the union of visible keys.

The map has a `shared_mutex`: allocation takes a unique lock, while lookup and
key enumeration take shared locks. `getKeys()` holds its lock while traversing
the current map. However, `get()` and `contains()` unlock before they finish
using the iterator returned by `find()`. As currently written, they are not
safe against a concurrent `alloc()` that rehashes the map. Even where map
access is synchronized, the lock does not protect later mutation of the
pointed-to `T`; code sharing a `Value` or `Resource` needs separate
synchronization for that object.

`getVarTable()` returns a mutable reference without holding a lock. Compiler
and linker code use it for cloning, duplicate checks, and inspection (including
`GraphLinker::getResources()`), but callers must not treat it as a
concurrency-safe mutation API.

## Compiler scopes

[`GraphLinker`](../../../src/compiler/graphLinker.cpp) begins with built-in resource names. Encountering a `BlockExpression` creates a child `Scope<Resource>` and pushes its flattened lifetime. `updateScopeLifetimes()` restores the enclosing scope when the count expires.

Functions require a lexical snapshot. [`cloneResourceScope()`](../../../src/compiler/resource.cpp) recursively clones scope structure and resource declarations while reinitializing access state. `enterFunction()` saves the outer working scope and uses a clone; deferred recursive linking clones the function's saved scope again so incomplete first-pass access state does not leak into replacement summaries.

## Runtime scopes

The executor assigns `Scope<Value>` objects to instructions. A block receives a child scope, and nested instructions share the appropriate scope. Function invocation clones a subprogram and creates invocation-local parameter state while retaining lexical access to enclosing values. `ReferenceIdentifier` returns the shared stored value for assignment; `GetIdentifier` reads it.

The distinction is important: compiler resources describe ordering and function capture, but contain no user value. Runtime values contain data, but do not rebuild the dependency graph.

## Concrete observed examples

I ran:

```sh
build/Tests --gtest_filter='get.*:alloc.*:linkGraph.allowsVariableShadowing:E2E/E2EFixture.E2E/NestedBlocksCanShadowEnclosingVariables1:E2E/E2EFixture.E2E/CallsWithArgumentsDoNotShareParameterScopes16'
```

Observed: all selected tests passed. `alloc.canShadowEnclosingVar` confirmed current-scope lookup wins. The nested-block E2E program declares outer `a = 1`, inner `a = 2`, and prints both values; observed output satisfied `{1, 2}`. The 16-worker call test confirmed concurrent calls do not share parameter scopes.

## Invariants and hazards

- Shadowing allocates in the current scope; assignment resolves the nearest existing binding.
- Scope objects and stored objects use shared ownership because instruction graphs and function captures outlive local parser/linker stack frames.
- Keep compiler scope lifetime counts aligned with flattened instruction counts.
- Do not call `get()`, `contains()`, or use a `getVarTable()` reference while
  another thread may call `alloc()` on the same scope; the current iterator
  lifetime is not protected through lookup completion.
- `getKeys()` merges names, not binding identities; use `get(name)` pointer equality when determining which lexical binding is meant.
- New runtime parallelism must audit mutation of pointed-to `Value` objects separately from the map's mutex.
