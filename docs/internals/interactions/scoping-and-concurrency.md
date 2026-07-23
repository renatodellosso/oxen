# Scoping and concurrency

Both compiler and runtime use the template
[`Scope<T>`](../../../src/scope.hpp), but with different payloads. The compiler
uses `Scope<Resource>` while linking lexical dependencies in
[`GraphLinker`](../../../src/compiler/graphLinker.hpp); the executor uses
`Scope<Value>` for storage in
[`Executor::initScopes` and block/call execution](../../../src/interpreter/executor.cpp).

## Runtime scope construction

`initScopes()` creates a root scope containing `int`, `bool`, and `string`, then
a global child assigned to every root instruction. Executing a `Block` creates
a child of the block instruction's current scope and assigns it to direct body
instructions, skipping nested block ranges because those construct their own
children.

Executing a function declaration stores a `Function` value in the current
scope. A call clones the function body, then replaces the clone's block scope
with a fresh child before assigning that scope to argument declaration
instructions. This provides parameter shadowing and isolates simultaneous call
arguments while retaining access to captured enclosing values.

## What `Scope` synchronizes

`alloc()` takes an exclusive `shared_mutex` lock around insertion/replacement
in the local `unordered_map`. `get()` and `contains()` locate an iterator under
a shared lock, release the lock, and then inspect that iterator; `getKeys()`
keeps its shared lock while copying local keys. `get()` returns the stored
`shared_ptr<T>`.

Because `get()` and `contains()` use an iterator after unlocking, the current
implementation does **not** make concurrent lookup safe against an `alloc()`
that rehashes the same table. The dependency graph normally orders declarations
before their uses, but code must not treat `Scope` as a general concurrent map.

The lock protects the map structure, not the object behind the returned
pointer. For `Scope<Value>`, `Set` writes `result->type` and `result->val`
without a `Scope` lock. Safety therefore depends on compiler-generated resource
dependencies ordering accesses to the same lexical variable. The shared pointer
keeps storage alive; it does not make `Value` mutation atomic.

## Invariants and change hazards

1. Preserve one fresh block/parameter scope per invocation. Reusing a call scope
   shares parameter `Value` objects across concurrent calls.
2. A declaration shadows by inserting into the current table; lookup must check
   local state before enclosing scopes.
3. Do not mistake `Scope`'s mutex for a complete concurrent-map guarantee or
   for protection of returned values. Any new runtime path that mutates a
   `Value` must participate in resource dependency analysis or introduce
   explicit value-level synchronization.
4. `getVarTable()` returns an unlocked mutable reference. It is unsuitable for
   concurrent access; prefer `alloc()` and `getKeys()`, and use `get()` or
   `contains()` only when insertion into that scope cannot race.
5. `getEnclosing()` and `getDepth()` assume the enclosing pointer is immutable
   after construction.
6. Compiler `Resource` and runtime `Value` scopes must make matching shadowing
   choices. A disagreement links dependencies to one variable but mutates
   another at execution.

## Observed example

```ox
int value = 10;
void show(int value) { print "parameter=" + value; }
show(20);
print "global=" + value;
```

```sh
build/CLI -t scope.ox -h 8
```

Observed:

```text
parameter=20
global=10
```

The values demonstrate parameter shadowing without changing the global; output
order is free because these reads do not mutate the variables. Block-scope and
lookup coverage is in [`tests/scope.cpp`](../../../tests/scope.cpp), and
[`FunctionParametersShadowVariablesWhenCalled`](../../../tests/e2e/tests.cpp)
guards compiler/runtime agreement.
