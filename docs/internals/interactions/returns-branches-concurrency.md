# Returns, branches, and concurrent calls

Branch skipping and return claiming meet in [`Executor::updateDependency`](../../../src/interpreter/executor.cpp#L193). All cloned return sites share one invocation-local [`ReturnInvocation`](../../../src/instruction.hpp#L65), but each concurrent call has a distinct object and ID. Untaken return markers remain argument-indexed, so recursive skipping does not claim the invocation with `nullptr`.

For:

```parallel
string choose(bool outer, bool inner) {
  if (outer) return "outer";
  else { if (inner) return "inner"; else return "fallback"; }
}
```

the focused [`isolatesBranchedReturnsAcrossConcurrentCallInvocations`](../../../tests/interpreter/executor.cpp#L466) launches four calls per branch (12 total) with 16 workers and requires exactly four of each result. The 16-worker [`IfStatementsCanRunInsideElseBlocks`](../../../tests/e2e/tests.cpp#L638) covers nested branch returns. Calling the documented function as `choose(true, false)`, `choose(false, true)`, and `choose(false, false)` produces `outer`, `inner`, and `fallback` (the print order is unconstrained).

Invariant: first executed return wins per invocation, not per static call site or function. Never attach `claimed` state to reusable `InstrDependent` objects; that allows one worker's call to consume another's return.
