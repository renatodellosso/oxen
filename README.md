# Oxen, an Automatically Parallel Programming Language

Oxen is a programming language that automatically multithreads your code. For example, consider the following program:

```
int x = 1;
int y = 2;

print x;
print y;
```

The declarations and print statements will run in parallel, across multiple threads.

## Commands

You can compile Oxen into the `build` folder with `cmake --build build`. Once you've done so, the following commands are available:

 - `build/CLI -t <source>.ox -h <thread count, defaults to 1>`: Compiles and runs the Oxen source file `<source>.ox`.
 - `build/CLI -c <source>.ox -o <output file>`: Compiles the Oxen source file `<source>.ox` into bytecode and writes it to `<output file>`.
 - `build/CLI -i <bytecode file> -h <thread count, defaults to 1>`: Interprets the bytecode file `<bytecode file>`.
 - `build/Tests`: Runs the test suite.
 - `build/Benchmark`: Runs the benchmark suite.

## How does it work?

Oxen compiles Oxen source code into bytecode, which is then interpreted by the Oxen interpreter. Programs are transformed into graphs, with instructions being vertices and dependencies between them being edges. The graph is converted to a bytecode format, which can then be interpreted.

## Language Features

Oxen supports the following language features:

 - Arithmetic
 - Variables
 - Conditional (if/else statements)
 - Loops
 - Functions (including recursion)
 - Returns (note the returns in Oxen do NOT end the function, though only the first return statement will take effect)
 - Console output

## Syntax

Lines in Oxen end with `;`. Variables are declared as `<type> <name>` and an optional initializer value (append `= <initial value>`). The currently supported types are `int`, `string`, and `bool`.

Values can be printed to console with `print <value>`. Conditional statements are written as `if (<condition>) { <body> } else { <body> }`. Loops are written as `while (<condition>) { <body> }`. Functions are declared as `<return type> <name>(<parameters>) { <body> }` and called as `<name>(<arguments>)`. Return statements are written as `return <value>;`.

See below for an example:

```
int a;
a = 2;
int b = 5;

bool isPositive(int x) {
  if (x > 0) {
    return true;
  } else {
    return false;
  }
}

print "a is positive: " + isPositive(a);

while (isPositive(b)) {
  print "b is: " + b;
  b = b - 1;
}
```

## Docs

There's a set of AI-generated documentation for Oxen's internals in the `docs` folder. It's meant as an aid to myself when working on Oxen, so it's not the most polished.
