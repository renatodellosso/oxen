# Tokenizer contracts

[`Tokenizer`](../../../src/compiler/tokenizer.cpp) is a single-pass stream tokenizer. `parse()` repeatedly calls `parseToken()` until EOF; `close()` moves out the owned token vector. Tokens retain raw spelling and a one-based source line.

## Classification

Single-character punctuation/operators are emitted immediately. `!`, `=`, `<`, and `>` optionally consume `=`. A `-` followed immediately by a digit begins an integer literal; otherwise it is `Minus`. Strings begin at `"`, retain both quotes in `Token::raw`, and may contain whitespace/newlines. Integers consume decimal digits only. All remaining starts take the identifier path, whose continuation accepts ASCII letters and digits.

After collecting a word, exact raw values classify `true`/`false` as bool literals and `if`, `else`, `while`, `print`, and `return` as keywords. Everything else is `Identifier`. The enums and payload layout are in [`token.hpp`](../../../src/compiler/token.hpp).

`skipWhitespace()` increments `line` for either `\n` or `\r`. A CRLF sequence therefore counts as two lines; callers and diagnostics currently inherit that behavior. Newlines inside strings also advance the line counter.

## Important lexical consequences

- `-123` is one literal, but `- 123` is a subtraction token followed by a literal.
- There is no comment syntax in source tokenization.
- There is no escape decoding. The tokenizer retains raw string contents. Its current loop stops either after appending `"` or after appending any character whose immediately preceding character is `\\`; consequently, a backslash followed by a non-quote character terminates the string token early. This is implementation behavior, not a supported escape grammar.
- Identifier continuation excludes `_`, although an unrecognized initial
  character takes the identifier path. Do not assume a conventional C
  identifier grammar without changing and testing it explicitly.
- Malformed/unrecognized characters are not rejected here: the default path produces identifiers and lets AST construction report contextual errors.

## Concrete observed examples

The following command runs the focused tokenizer examples:

```sh
build/Tests --gtest_filter='Tokenizer.*'
```

Observed: all 23 tokenizer tests passed. In particular:

- `Tokenizer.identifiesNegativeNumbers` observed one integer-literal token with raw `-123`.
- `Tokenizer.doesNotConsumeTokensAfterEmptyStrings` observed `""` followed by `)` as two tokens.
- `Tokenizer.tracksLineNumbers` tokenized `;\n;;\r;` at lines `1, 2, 2, 3`.
- `Tokenizer.tracksLineNumbersInMultilineStrings` observed the token after a newline-containing string at line 2.

## Change checklist

When adding syntax, update `TokenType`, `Tokenizer::parseToken()`, focused tests
in [`tests/compiler/tokenizer.cpp`](../../../tests/compiler/tokenizer.cpp), and
the `AstBuilder` match/parse path. Test adjacency, whitespace, EOF, malformed
input, and line accounting. A raw-string representation change also requires
auditing
`RootExpression::toByteCode()` and `BytecodeParser::buildArg()`, because
serialization currently relies on quoted raw spelling.
