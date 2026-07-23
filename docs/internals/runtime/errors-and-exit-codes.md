# Error boundaries and exit codes

Public process outcomes are defined by
[`ExitCode`](../../../src/exitCode.hpp). Because the enum has implicit values,
the current numeric codes are `Ok=0`, `InvalidCli=1`, `SyntaxErrors=2`,
`FailedToWriteFile=3`, `BytecodeParseError=4`, and `ExecutionError=5`.
[`Error`](../../../src/error.hpp) carries an `ExitCode` across an exception
boundary.

## Boundaries

| Stage | Boundary | Result |
|---|---|---|
| failed CLI validation | [`runCli`](../../../src/cli.cpp) | `InvalidCli` |
| AST building or graph linking | [`compile`](../../../src/compiler/compiler.cpp) | logs all syntax errors, returns `SyntaxErrors` |
| bytecode output callback | `compile` | `FailedToWriteFile` |
| bytecode construction | [`executeBytecode`](../../../src/interpreter/interpreter.cpp) | catches `std::runtime_error`, throws `Error(BytecodeParseError, ...)` |
| executor | `executeBytecode` | catches `std::runtime_error`, throws `Error(ExecutionError, ...)` |
| interpreter API | [`Interpreter::interpret`](../../../src/interpreter/interpreter.cpp) | catches `Error`, logs it, returns its code |
| benchmark process | [`runApplication`](../../../benchmark/application.cpp) | catches `std::exception`, writes to stderr, returns 1 |

Workers catch runtime errors in `Executor::execWorker()`, record a diagnostic in
`haltCause`, set `failed` and `halt`, and let `startExecution()` rethrow after
workers join. This converts a worker failure into a caller-visible execution
error.

## Invariants and change hazards

- Catch at application/API boundaries, not inside individual operators where
  swallowing the error could release invalid dependents.
- Preserve the original cause and instruction ID when wrapping errors.
- If inserting or reordering `ExitCode` members, assign explicit numeric values
  first or existing shell/API contracts will silently change.
- `executeBytecode()` exposes typed `Error`; `Interpreter::interpret()` converts
  it to a return code. Callers must choose the appropriate boundary.
- The parser currently validates little and uses unchecked indexing in
  `BytecodeParser::buildDependents()`. Some malformed bytecode can escape the
  intended `std::runtime_error` wrapper or access an invalid instruction. Treat
  `BytecodeParseError` as the designed boundary, not proof that arbitrary input
  is safely rejected.
- The interpret-only path in `executeCommand()` currently calls
  `Interpreter::interpret()` without returning its result, then falls through
  to `InvalidCli`. This is a known implementation hazard documented in
  [CLI and shared APIs](../tooling/cli-and-shared-apis.md).

## Observed examples

A runtime failure:

```ox
print 1 / 0;
```

```sh
build/CLI -t /tmp/parallel-runtime-error.ox -h 2
```

```text
[Error at Executor:worker1]: [instruction 2] Division by zero on instruction 2
[Error at Interpreter]: Encountered error executing bytecode: Runtime Error in worker 1 [instruction 2]: Division by zero on instruction 2
# process exit code: 5
```

A compile-time syntax failure:

```ox
if (true) { print 1;
```

```sh
build/CLI -t /tmp/parallel-syntax-error.ox
```

```text
[Error at compiler]: Found 1 syntax errors while building AST
[Error at compiler]: Syntax error at line 1: Expected '}' after block!
# process exit code: 2
```

Exact E2E expectations for both categories live in
[`tests/e2e/tests.cpp`](../../../tests/e2e/tests.cpp); the typed wrapper has a
unit test in [`tests/error.cpp`](../../../tests/error.cpp).
