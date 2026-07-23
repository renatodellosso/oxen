# C++ and software-engineering hardening roadmap

This document records the highest-value practices and technical improvements
for making Oxen's C++ code safer, easier to review, and easier to extend.
It is a prioritized roadmap, not a request to rewrite the project at once.

The project already has important strengths: C++20, cross-platform CI, focused
compiler and interpreter tests, parameterized multi-worker E2E coverage, and
detailed internal documentation. The next step is to make ownership,
synchronization, and invalid-input behavior explicit enough that the compiler
and automated tooling can enforce them.

## Engineering principles

Apply these questions during design and review:

1. Who owns every object, and can a non-owning reference outlive it?
2. What invariant does each constructor establish?
3. Which mutex protects each mutable field?
4. What happens for empty, malformed, overflowing, or adversarial input?
5. Can a type or API make an invalid state unrepresentable?
6. Can a warning, sanitizer, static check, or focused test enforce the
   assumption?
7. Is a comment explaining a design constraint, or compensating for an API that
   is too easy to misuse?

The C++ Core Guidelines are the baseline reference for ownership, polymorphism,
RAII, error handling, and concurrency:
[C++ Core Guidelines](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines).

## Priority summary

| Priority | Work | Primary risk addressed |
|---|---|---|
| P0 | Safe polymorphic destruction | undefined behavior and incomplete destruction |
| P0 | Repair `Scope` locking boundaries | iterator invalidation and data races |
| P0 | Validate worker count and propagate worker failures safely | hangs, termination, and races in error reporting |
| P0 | Initialize every field | indeterminate scheduler and compiler state |
| P1 | Clarify ownership and remove strong-reference cycles | leaks and hidden lifetime coupling |
| P1 | Add warnings, sanitizers, static analysis, and working CTest registration | defects hidden by the normal build |
| P1 | Validate bytecode and CLI input | out-of-bounds access and accidental invalid states |
| P2 | Separate immutable program data from runtime state | difficult synchronization and unsafe reuse |
| P2 | Replace polling with predicate-based queue waiting | wasted CPU and fragile shutdown behavior |
| P2 | Improve the CMake target structure | duplicate compilation and inconsistent target settings |
| P3 | Split large responsibilities and tighten APIs | review cost and regression risk |
| P3 | Add fuzzing, stress, and hermetic test infrastructure | schedule- and input-dependent defects |

## P0: immediate correctness work

### Make polymorphic destruction safe

[`Expression`](../../src/compiler/expression.hpp) has virtual functions and is
owned through base-class smart pointers, but it does not declare a virtual
destructor. Conversions from a derived `unique_ptr` to `unique_ptr<Expression>`
occur in [`AstBuilder`](../../src/compiler/astBuilder.cpp). Destroying a derived
object through a base pointer without a virtual destructor is undefined
behavior.

Add a public virtual defaulted destructor if destruction through `Expression`
is supported:

```cpp
virtual ~Expression() = default;
```

Derived leaf classes can be marked `final` where further inheritance is not
part of the design. Avoid mixing a polymorphic hierarchy, unchecked
`static_cast`s, and a separately mutable `InstructionType` tag unless the code
has one well-tested invariant keeping all three consistent. For a closed set of
AST node types, a `std::variant` plus visitors is an alternative; otherwise keep
the hierarchy and make virtual dispatch the authoritative mechanism.

Before changing production code, add a unit test with a derived expression
whose destructor has an observable effect. Preserve the behavior through an E2E
compile-and-run case and verify the relevant tests with ASan/UBSan.

### Keep `Scope` iterators inside the lock lifetime

[`Scope::get()` and `Scope::contains()`](../../src/scope.hpp) find an iterator,
unlock the `shared_mutex`, and then compare or dereference that iterator. A
concurrent insertion can rehash the `unordered_map` and invalidate it.
`getVarTable()` also returns an unlocked mutable reference to the protected map,
which bypasses the class's synchronization policy.

The desired contract is:

- copy the `shared_ptr<T>` result while the shared lock is held;
- release the lock before recursively querying an enclosing scope;
- do not expose mutable container references;
- provide a locked snapshot or a purpose-specific operation instead;
- make read-only operations `const`, using a `mutable` mutex when necessary;
- treat `enclosing` as immutable after construction, or protect it explicitly.

Add a focused unit test with concurrent readers and insertions before the fix,
then run it under TSan. Add an E2E case involving concurrent access to lexical
scope if the defect is observable through the language.

### Validate worker count before execution

[`--threads`](../../src/cli.cpp) is currently parsed with `atoi`. Invalid input
can become zero, and an executor with queued work but no worker threads can wait
forever in its supervisor.

Parse with `std::from_chars`, require complete consumption, and enforce a
documented range such as `1..maxWorkerCount`. Perform validation at the CLI
boundary and retain a constructor precondition in `Executor` so non-CLI callers
cannot bypass it.

Add CLI unit cases for zero, negative, nonnumeric, overflowing, and missing
values. Add an E2E invocation that confirms invalid thread counts fail promptly
with the intended exit code rather than hanging.

### Make the worker exception boundary total

[`Executor::execWorker()`](../../src/interpreter/executor.cpp) catches only
`std::runtime_error` by value. Other exceptions escaping a thread cause
`std::terminate`. Multiple workers can also attempt to update shared failure
text concurrently.

At the thread boundary:

1. catch `...`;
2. capture `std::current_exception()`;
3. publish only the first failure under a mutex or `std::once_flag`;
4. request shutdown;
5. join every worker;
6. rethrow on the calling thread.

Elsewhere, throw by value and catch exception hierarchies by `const` reference.
Test one `runtime_error` and one non-`runtime_error` failure, including a
multi-worker case that permits simultaneous failure.

### Initialize every member

Every constructor should establish a usable invariant. Prefer default member
initializers for scalar state and override them only when a constructor needs a
different value. For example, review the scheduler fields on
[`Instruction`](../../src/instruction.hpp), including `executed`, and compiler
state such as `FunctionExpression::deferred`.

Avoid using casts to manufacture default enum values. Give enums an explicit
invalid value when an object legitimately has a pre-parse state, or require the
real value in the constructor. Enable compiler warnings for missing fields and
member initialization order.

## P1: ownership and boundary hardening

### Express one clear owner

Use these conventions consistently:

- value: owned directly and cheap or natural to copy;
- `unique_ptr<T>`: one owner with heap-stable identity;
- `shared_ptr<T>`: multiple independent owners that genuinely extend lifetime;
- `weak_ptr<T>`: non-owning observation of shared ownership;
- `T&`: required, non-null, non-owning access;
- `T*`: optional or reseatable, non-owning access;
- stable ID: preferred graph relationship when identity can be resolved by an
  owning arena or vector.

The compiler AST is conceptually an owning tree with non-owning dependency
edges. Prefer unique ownership for tree edges unless sharing is essential, and
represent dependency relationships with stable node IDs or documented
non-owning references. Do not pass smart pointers merely because the caller
uses one; pass `T&` or `const T&` when a function does not participate in
lifetime management.

The runtime currently forms a strong-reference cycle:

```text
Subprogram -> vector<Instruction> -> Instruction::program -> Subprogram
```

See [`Subprogram`](../../src/interpreter/subprogram.hpp) and
[`Instruction`](../../src/instruction.hpp). Make the backlink non-owning or
replace it with stable program/instruction IDs. Remove the two-phase "construct,
then remember to call `setSubprogramPointers()`" protocol; a constructor or
factory should return an object whose invariants are already established.

Add a lifetime unit test using `weak_ptr` observation or a sanitizer leak check,
plus an E2E function-call case to protect pointer repair and cloned-call
behavior.

### Define the bytecode trust boundary

[`BytecodeParser`](../../src/interpreter/bytecodeParser.cpp) currently relies on
`atoi`, unchecked dependent IDs, numeric enum casts, and assumed opcode argument
shapes. The CLI can interpret a file directly, so either bytecode must be
validated as external input or the CLI must explicitly enforce a trusted-input
contract.

For a robust file format:

- parse integers with `std::from_chars` and check overflow and complete input;
- reject unknown opcodes before constructing an instruction;
- check dependency IDs and argument indices before indexing;
- validate opcode-specific argument counts and types;
- report line, field, and reason in parse diagnostics;
- add a format version before changing opcode identities;
- use explicit serialized opcode values rather than enum declaration order.

Add table-driven unit tests for empty, truncated, malformed, negative,
out-of-range, and oversized fields. Add an E2E direct-interpretation case for
each user-visible error category.

## P1: automated quality gates

### Compiler warnings

Apply warnings to project targets, not third-party dependencies. Start with:

```text
Clang/GCC: -Wall -Wextra -Wpedantic
MSVC:      /W4
```

Then clean up and enable `-Wconversion` and `-Wsign-conversion`. Promote the
agreed warning baseline to errors in CI (`-Werror` or `/WX`). Use consistent
index and count types, and validate conversions where bytecode's signed offsets
meet container sizes.

### Sanitizers and static analysis

Add separate CI configurations:

- ASan + UBSan Debug build, with frame pointers;
- Linux TSan build for executor, scope, queue, and E2E concurrency tests;
- `clang-tidy` using a checked-in `.clang-tidy` configuration;
- optional source coverage as a report, not as a substitute for correctness.

Useful initial clang-tidy groups are `bugprone-*`, `clang-analyzer-*`,
`concurrency-*`, `performance-*`, and a curated subset of
`cppcoreguidelines-*`. Introduce checks incrementally and require an explanation
for each suppression.

Official tool references:

- [AddressSanitizer](https://clang.llvm.org/docs/AddressSanitizer.html)
- [UndefinedBehaviorSanitizer](https://clang.llvm.org/docs/UndefinedBehaviorSanitizer.html)
- [ThreadSanitizer](https://clang.llvm.org/docs/ThreadSanitizer.html)
- [clang-tidy](https://clang.llvm.org/extra/clang-tidy/)

### Register tests with CTest

The project calls `gtest_discover_tests()` but does not currently enable CTest;
`ctest --test-dir build -N` therefore reports no registered tests. Add
`include(CTest)` or `enable_testing()`, retain direct `build/Tests` support, and
use this CI command for useful failure output and test-runner integration:

```sh
ctest --test-dir build --output-on-failure
```

## P2: concurrency architecture

### Separate graph definition from execution state

An instruction currently combines serialized program data with mutable
scheduler state. Move toward two models:

```text
ProgramGraph (immutable after validation)
  opcode, bytecode arguments, dependency IDs

ExecutionState (owned by one execution/invocation)
  dependency arguments, fulfilled count, queued/skipped state, result
```

This makes concurrent reads cheap, prevents one invocation from leaking state
into another, and lets mutexes live beside the state they protect. It also
clarifies whether an executor can be reused: either construct fresh execution
state or make the type explicitly single-use.

Introduce this incrementally around the most failure-prone call, return, and
loop-reset state. Preserve the focused executor and E2E coverage described in
[concurrency regression testing](../internals/maintenance/concurrency-regression-testing.md).

### Replace polling with predicate-based waiting

[`ConcurrentQueue`](../../src/concurrentQueue.hpp) returns an empty sentinel,
and workers and the supervisor sleep and retry. Replace this with a queue that
owns:

- its mutex;
- a condition variable;
- queued items;
- a closed or stop state.

Workers should wait on a predicate such as `closed || !queue.empty()`. Prefer
`std::jthread` and `stop_token` where they simplify joining and cancellation.
Do not use sleep duration as a correctness mechanism.

Add real multi-producer/multi-consumer queue tests using barriers or latches,
then run them repeatedly and under TSan. Keep timing assertions out of the
correctness suite.

### Document synchronization contracts

For each shared field, document the guarding mutex next to the field. Establish
and enforce a lock order for operations that need more than one lock. Avoid
`recursive_mutex` where a locked and unlocked helper can express recursion more
clearly. Snapshot diagnostic state under its locks and format or write logs
after releasing them so verbose mode cannot introduce races or long critical
sections.

## P2: CMake and dependency structure

Compile production code once through a library target:

```text
parallel_core
|- CLI
|- Tests
`- Benchmark
```

Create a separate `benchmark_lib` if benchmark implementation needs unit tests.
Attach language level, warnings, include directories, sanitizer options, and
definitions to the narrowest appropriate target.

Additional changes:

- use `project(... LANGUAGES CXX)`;
- use `target_compile_features(... cxx_std_20)`;
- set `CMAKE_CXX_EXTENSIONS OFF`;
- list sources explicitly instead of recursive globs;
- pin GoogleTest to a reviewed commit hash rather than `main`;
- keep sanitizer and warning options configurable through CMake options or
  presets;
- keep benchmark measurements separate from correctness gating on shared CI
  runners.

CMake recommends explicit source lists and commit hashes for remotely fetched
dependencies:
[file(GLOB) guidance](https://cmake.org/cmake/help/latest/command/file.html#glob)
and [FetchContent guidance](https://cmake.org/cmake/help/latest/module/FetchContent.html).

## P3: APIs, modules, and readability

### Split by responsibility

[`Executor::execSingleInstruction()`](../../src/interpreter/executor.cpp) and
[`GraphLinker::processExpression()`](../../src/compiler/graphLinker.cpp) encode
many distinct policies. Split them by stable responsibility rather than an
arbitrary line limit:

- arithmetic and comparison evaluation;
- scope and variable operations;
- branch and loop control;
- call setup and completion;
- return selection;
- dependency publication;
- bytecode remapping parsing and validation.

Each extracted component should have a narrow interface and focused unit tests.
Avoid a class per opcode if that only scatters one coherent rule across many
files.

### Tighten interfaces

- Make observers `const`.
- Use `std::string_view` for non-owning text input.
- Use `std::span<const T>` for non-owning contiguous input.
- Pass inexpensive values by value and larger read-only objects by `const&`.
- Return values or read-only views rather than mutable container references.
- Add `[[nodiscard]]` where ignoring a parse, validation, or pop result is a
  likely defect.
- Introduce strong types for instruction IDs, argument indices, bytecode
  offsets, invocation IDs, and worker counts when mixing them is dangerous.
- Put project code in a namespace and file-local helpers in anonymous
  namespaces.
- Make headers self-contained: include what each header uses and do not depend
  on transitive includes.

Use `clang-format` for mechanical layout and reserve review attention for
behavior, ownership, and invariants.

## P3: test strategy

Preserve the existing unit, compiler, interpreter, E2E, and benchmark layers.
Extend them in the areas ordinary functional tests cannot cover:

- repeated seeded concurrency stress tests;
- queue tests with actual concurrent producers and consumers;
- TSan executions of focused and E2E concurrency cases;
- parser fuzz targets seeded with valid and invalid source and bytecode;
- regression tests for every minimized sanitizer or fuzzing failure;
- hermetic temporary directories with RAII cleanup;
- explicit tests for ordered versus intentionally unordered side effects;
- property checks such as "all generated dependency IDs resolve" and
  "serialization followed by parsing preserves the validated graph."

LLVM's [libFuzzer](https://llvm.org/docs/LibFuzzer.html) can be combined with
ASan and UBSan for the tokenizer and bytecode parser. A fuzz target must accept
empty and malformed input without terminating the process.

Follow the repository rule for bug fixes: first add both the smallest useful
unit reproducer and an E2E reproducer, confirm they fail for the expected
reason, implement the fix, and then run the focused tests followed by
`build/Tests`. For concurrency defects, also follow the repetition and Release
build guidance in
[concurrency regression testing](../internals/maintenance/concurrency-regression-testing.md).

## Suggested delivery sequence

1. Add virtual destruction and initialization tests; fix those invariants.
2. Add concurrent `Scope` and invalid-thread-count reproducers; fix both.
3. Add total worker exception propagation and its tests.
4. Establish the warning baseline and make CTest registration work.
5. Add ASan/UBSan and a focused Linux TSan job.
6. Remove the `Subprogram` ownership cycle and two-phase initialization.
7. Harden bytecode parsing and version the format.
8. Introduce the blocking queue and RAII thread lifetime.
9. Separate immutable graph data from invocation-local execution state.
10. Restructure CMake targets and then split the largest implementation
    responsibilities behind tested interfaces.

Keep each change small enough to explain one invariant. Avoid combining a
correctness fix with broad mechanical refactoring unless the refactoring is
required to make the fix safe.

## Definition of done for each item

- the bug or missing invariant is demonstrated by a focused unit test and an
  E2E test where user-visible;
- production code makes ownership and synchronization visible in its types;
- relevant warning, sanitizer, and static-analysis configurations pass;
- focused Release stress tests pass for concurrency changes;
- the complete `build/Tests` suite passes;
- internal documentation is updated with the new invariant;
- the change description records exact build modes, commands, and observed
  results without treating a single concurrency run as proof of race freedom.
