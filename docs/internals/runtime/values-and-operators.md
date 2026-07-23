# Runtime values and operator dispatch

[`Value`](../../../src/value.hpp) is a tagged value containing a
`ValueType` and a variant of `string`, `int`, `bool`, or `shared_ptr<Function>`.
Identifiers also use the string variant. Literal parsing happens in
[`BytecodeParser::buildArg`](../../../src/interpreter/bytecodeParser.cpp);
operator behavior is dispatched in
[`Executor::execSingleInstruction`](../../../src/interpreter/executor.cpp).

## Copies, references, and conversion

`ReferenceIdentifier` returns the `shared_ptr<Value>` stored in a runtime
[`Scope`](../../../src/scope.hpp), so `Set` mutates the variable in place.
`GetIdentifier` instead creates a new `Value` copy for ordinary reads. This
distinction is what makes assignment update storage without allowing later
changes to retroactively alter values already sent down the graph.

[`valToStr`](../../../src/value.cpp) renders integers, lowercase booleans,
strings (optionally quoted), identifiers, and function descriptions.
`valToBool()` defines condition truthiness: nonzero integers and nonempty
strings are true, functions are true, and identifiers fall through to false.

## Operator table

| Instruction | Accepted values | Result |
|---|---|---|
| `Add` | two integers | integer sum |
| `Add` | either operand string | concatenated `valToStr` strings |
| `Add` | otherwise, either operand bool | boolean OR using truthiness |
| `Add` | other operand pairs | no result is produced (current implementation hazard) |
| `Subtract`, `Multiply`, `Divide` | two integers | integer arithmetic |
| `CompareEquals`, `CompareNotEquals` | same primitive type | value comparison |
| equality with different types | any differing tags | false / true |
| ordered comparisons | matching integers or strings | boolean comparison |

Division is truncating integer division and explicitly rejects zero.
`Subtract`, `Multiply`, `Divide`, and ordered-comparison type errors include the
instruction ID and numeric `ValueType` tags. `Add` is an exception: if neither
the integer, string, nor boolean branch matches, it leaves the result null
instead of throwing. Equality rejects matching nonprimitive types, but
different types compare unequal without an error.

## Invariants and change hazards

- Keep the `type` tag and variant alternative consistent. Most paths use
  `std::get`, so disagreement throws.
- String addition takes precedence over boolean addition. Changing branch order
  changes mixed expressions such as `true + "x"`.
- Boolean addition is OR, not numeric addition.
- Comparison support is intentionally asymmetric: strings can be ordered,
  booleans cannot.
- `InstructionType::Negate` exists in the enum and string formatter, but the
  executor has no `Negate` switch case. Negative numeric literals currently
  reach execution as literals; emitting `Negate` would hit the unknown-type
  runtime error.
- Independent expressions can print in any schedule order. Operator tests
  should compare results independently unless dependencies require order.

## Observed example

Source `/tmp/parallel-runtime-docs-values-20260723.ox` contained:

```ox
print "answer=" + 42 + ", enabled=" + true;
print 2 + 3;
print false + true;
print "ant" < "bee";
print 1 == true;
```

Command and observed output:

```sh
build/CLI -t /tmp/parallel-runtime-docs-values-20260723.ox -h 4
```

```text
5
false
true
true
answer=42, enabled=true
```

The values prove integer addition, mixed stringification, boolean OR, lexical
string ordering, and mixed-type equality. Their order is not a semantic
guarantee. Focused operator coverage is in
[`tests/interpreter/executor.cpp`](../../../tests/interpreter/executor.cpp) and
language-level combinations are in
[`tests/e2e/tests.cpp`](../../../tests/e2e/tests.cpp).
