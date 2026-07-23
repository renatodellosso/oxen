# Return selection

A `return` produces a value; it does not terminate the rest of the function body. [`FunctionExpression::findReturnStatements`](../../../src/compiler/expression.cpp) records every return for call serialization. At invocation creation, the executor gathers argument-indexed call dependents and creates one [`ReturnInvocation`](../../../src/instruction.hpp) with a unique ID, atomic `claimed` latch, and snapshot of destinations.

Every cloned return gets a marker edge carrying that invocation. [`Executor::updateDependency`](../../../src/interpreter/executor.cpp) uses `claimed.exchange(true)`: the first executed return forwards its result, and later returns are ignored. Returns are additionally chained in source order so a later reachable return cannot race ahead of an earlier one. The marker retains an argument index, preventing `skipInstruction` from treating an untaken return as a null result.

Invariant: “first executed return wins” independently for every call, while subsequent side effects still execute. Mutable selection state belongs to the cloned invocation, never the reusable call edge.

## Evidence

The direct example:

```parallel
int choose(bool first) {
  if (first) return 10; else return 20;
  print "side-effect";
}
print choose(true);
print choose(false);
```

run with `build/CLI -t <file> -h 16` produced one `10`, one `20`, and two
`side-effect` lines; their global order is unspecified. The focused
[`isolatesBranchedReturnsAcrossConcurrentCallInvocations`](../../../tests/interpreter/executor.cpp)
test also passed; it checks four concurrent calls for each of `outer`, `inner`,
and `fallback`.
[`ReturnsRunLaterSideEffectsWithoutOverwritingReturnValue`](../../../tests/e2e/tests.cpp)
passed at 16 workers.

Symptoms of broken isolation are duplicated fallback results, missing call results, or one invocation claiming another invocation's return.

## Change workflow

A contributor changing return lowering must keep the serialized return IDs in
`UnaryCallExpression::toByteCode()` aligned with
`parseReturnIdsFromBytecodeArgs()` in `executor.cpp`. A focused executor test
should create several simultaneous invocations with different reachable
branches and compare results without assuming global print order. The E2E
regression above separately verifies the language rule that statements after a
return still run.
