#include "compiler.hpp"
#include "../color.hpp"
#include "../logging.hpp"
#include "astBuilder.hpp"
#include "graphLinker.hpp"
#include "tokenizer.hpp"
#include <cmath>
#include <format>
#include <utility>

#define LOCATION "compiler"

ExitCode compileToBytecode(const CliArgs &args, std::istream &source,
                           std::string &bytecode) {
  return compile(args, source, [&bytecode](std::string text) {
    bytecode = std::move(text);
    return std::nullopt;
  });
}

ExitCode
compile(const CliArgs &args, std::istream &inputStream,
        std::function<std::optional<std::string>(std::string)> writeOutput) {
  bool shouldLog = args.verbose || args.mode == CliMode::Compile;

  Tokenizer *tokenizer = new Tokenizer(inputStream);

  tokenizer->parse();
  auto tokens = tokenizer->close();
  delete tokenizer;

  if (shouldLog)
    log(LOCATION, "Parsed {:d} tokens. Building AST...", tokens.get()->size());

  AstBuilder astBuilder(std::move(tokens));
  astBuilder.build();

  auto errors = astBuilder.getErrors();
  if (!errors->empty()) {
    logError(LOCATION, "Found {} syntax errors while building AST",
             errors->size());
    for (auto error : *errors.get()) {
      logError(LOCATION, "{}", error.toString());
    }

    return ExitCode::SyntaxErrors;
  }

  if (shouldLog)
    log(LOCATION, "Built AST");

  int exprId = 0;
  for (auto expr : *astBuilder.getExpressions().get()) {
    int orig = exprId;
    exprId = expr->numberExpressions(exprId);
  }

  if (shouldLog) {
    log(LOCATION, "Numbered expressions:");
  }

  GraphLinker graphLinker(args, astBuilder.getExpressions());
  graphLinker.linkGraph();

  errors = graphLinker.getErrors();
  if (!errors->empty()) {
    logError(LOCATION, "Found {} syntax errors while linking graph",
             errors->size());
    for (auto error : *errors.get()) {
      logError(LOCATION, "{}", error.toString());
    }

    return ExitCode::SyntaxErrors;
  }

  if (shouldLog)
    log(LOCATION, "Linked graph");

  // Renumber expressions
  exprId = 0;
  for (auto expr : *astBuilder.getExpressions().get()) {
    exprId = expr->numberExpressions(exprId);
  }

  if (shouldLog && args.verbose) {
    log(LOCATION, "-------- AST --------");

    for (auto expr : *astBuilder.getExpressions().get()) {
      log(LOCATION, "{}", expr->toString());
    }

    log(LOCATION, "------ END AST ------");
  }

  std::string bytecode = "";
  for (auto expr : *astBuilder.getExpressions().get()) {
    bytecode += expr->toByteCode(args) + "\n";
  }
  bytecode = bytecode.substr(0, bytecode.length() - 1); // Remove trailing '\n'
  auto result = writeOutput(bytecode);

  if (result.has_value()) {
    logError(LOCATION, "{}",
             colorize(std::format("Failed to write bytecode to file: {}",
                                  result.value()),
                      Color::Red));
    return ExitCode::FailedToWriteFile;
  }

  if (shouldLog)
    log(LOCATION, "{}", colorize("Wrote bytecode to file!", Color::Green));
  return ExitCode::Ok;
}
