# Runtime call execution

The `Call` case in [`Executor::execSingleInstruction`](../../../src/interpreter/executor.cpp) runs under `dependencyStateMutex` while it constructs invocation-local wiring. It retrieves the function value, clones [`Function::getBody`](../../../src/interpreter/function.cpp) through [`Subprogram::clone`](../../../src/interpreter/subprogram.cpp), allocates a fresh child scope for the cloned root block, and assigns a monotonically increasing invocation ID.

The executor then:

1. parses dependent remaps and copies the caller dependents;
2. gates the cloned body on every argument value and maps values to parameter first-use instructions;
3. assigns parameter declarations to the invocation scope;
4. attaches invocation-local return selection;
5. computes stable terminal completion signals and installs barriers;
6. enqueues the cloned root only after all wiring is complete.

The original `Call` suppresses normal dependent publication (`updateDeps=false`). Every release is explicit because return values, argument initialization, resource effects, and terminal completion have different destinations.

## Evidence

This program exercises returns, post-return effects, arguments, and calls in a
loop:

```parallel
int choose(bool first) {
  if (first) return 10;
  else return 20;
  print "side-effect";
}
void show(int value) { print "loop-call=" + value; }
print choose(true);
print choose(false);
int i = 0;
while (i < 3) {
  show(i);
  i = i + 1;
}
```

Running it with `build/CLI -t <file> -h 16` produced both `10` and `20`, two
`side-effect` lines, and ordered `loop-call=0`, `1`, `2`. Global ordering of
the independent groups is intentionally unspecified.

The focused executor run passed [`isolatesBranchedReturnsAcrossConcurrentCallInvocations`](../../../tests/interpreter/executor.cpp), which launches twelve branched calls with 16 workers, and [`waitsForCallsInsideLoopIterations`](../../../tests/interpreter/executor.cpp).

Failure modes include enqueueing before argument edges exist, sharing the template body's scope, or publishing the original call's dependents in addition to remapped edges.
