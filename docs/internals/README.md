# Oxen internals

These documents describe the implementation contracts that must be preserved
when changing the compiler, bytecode, dependency graph, or concurrent runtime.
They focus on data flow, ownership, completion, and cross-feature behavior
rather than user-facing language syntax.

Concrete examples in the documents were run against the checkout in which the
documentation was written. Timing is illustrative; graph structure, output,
exit codes, and instruction counts are the durable evidence. Known defects
found during the runs are identified as current behavior, not as intended
contracts.

## Suggested reading paths

For a compiler change, start with [the pipeline](architecture/pipeline.md),
[expression and instruction models](architecture/expression-instruction-models.md),
[GraphLinker](compiler/graph-linker.md), and [bytecode serialization](bytecode/serialization-and-identity.md).

For control flow or functions, start with
[completion semantics](architecture/completion-semantics.md), then read
[while lowering](control-flow/while-lowering.md),
[call compilation](functions/call-compilation-and-remapping.md), and
[call completion barriers](functions/call-completion-barriers.md).

For concurrency work, start with
[executor scheduling](runtime/executor-scheduling.md),
[invocation-local state](runtime/invocation-local-state.md),
[executor/control-flow mutation](interactions/executor-and-control-flow-mutation.md),
and the [regression-testing guide](maintenance/concurrency-regression-testing.md).

## Architecture

- [Compilation and execution pipeline](architecture/pipeline.md)
- [Expression and instruction object models](architecture/expression-instruction-models.md)
- [Completion semantics](architecture/completion-semantics.md)

## Compiler and bytecode

- [Tokenizer contracts](compiler/tokenizer.md)
- [AST parsing and expression extension](compiler/ast-parsing.md)
- [Resource dependency analysis](compiler/resource-dependencies.md)
- [GraphLinker traversal and state](compiler/graph-linker.md)
- [Bytecode format](bytecode/format.md)
- [Serialization and stable identity](bytecode/serialization-and-identity.md)

## Control flow

- [Branch lowering and execution](control-flow/branches.md)
- [Branch resource snapshots and merging](control-flow/branch-resource-merging.md)
- [While-loop lowering](control-flow/while-lowering.md)
- [Runtime loop reset](control-flow/runtime-loop-reset.md)
- [Instruction skipping](control-flow/instruction-skipping.md)

## Functions and calls

- [Function declarations and lexical capture](functions/declarations-and-captures.md)
- [Deferred and recursive linking](functions/deferred-and-recursive-linking.md)
- [Call compilation and dependency remapping](functions/call-compilation-and-remapping.md)
- [Runtime call execution](functions/runtime-call-execution.md)
- [Return selection](functions/return-selection.md)
- [Call completion barriers](functions/call-completion-barriers.md)

## Runtime

- [Executor scheduling and dependency publication](runtime/executor-scheduling.md)
- [Invocation-local versus reusable state](runtime/invocation-local-state.md)
- [Lexical scopes](runtime/lexical-scopes.md)
- [Values and operator dispatch](runtime/values-and-operators.md)
- [Output serialization](runtime/output-serialization.md)
- [Errors and exit codes](runtime/errors-and-exit-codes.md)

## Feature interactions

- [Branches and resources](interactions/branches-and-resources.md)
- [Branches and loops](interactions/branches-and-loops.md)
- [Loops and calls](interactions/loops-and-calls.md)
- [Calls and resource side effects](interactions/calls-and-side-effects.md)
- [Returns, branches, and concurrent calls](interactions/returns-branches-concurrency.md)
- [Nested calls and completion barriers](interactions/nested-calls-and-completion.md)
- [Recursion, deferred linking, and scope](interactions/recursion-linking-and-scope.md)
- [Loops and dependency remapping](interactions/loops-and-remapping.md)
- [Control flow and instruction skipping](interactions/control-flow-and-skipping.md)
- [Unordered structures and deterministic bytecode](interactions/unordered-structures-and-bytecode.md)
- [Executor concurrency and control-flow mutation](interactions/executor-and-control-flow-mutation.md)
- [Scoping and concurrency](interactions/scoping-and-concurrency.md)
- [CLI, tests, and benchmarks](interactions/cli-tests-and-benchmarks.md)

## Tooling and maintenance

- [CLI modes and shared APIs](tooling/cli-and-shared-apis.md)
- [Adding an instruction](maintenance/adding-an-instruction.md)
- [Adding or changing control flow](maintenance/changing-control-flow.md)
- [Concurrency regression testing](maintenance/concurrency-regression-testing.md)
- [Test architecture and coverage map](maintenance/test-architecture.md)
- [Benchmark architecture and interpretation](maintenance/benchmarking.md)

## Documentation standard

When implementation behavior changes, update the relevant document with:

- the invariant being changed;
- the source methods and owning data structures;
- a compact source-to-bytecode or source-to-runtime example;
- the exact command used to verify the example;
- the focused unit, graph, executor, and E2E tests that protect it;
- concurrency or compatibility hazards introduced by the change.

