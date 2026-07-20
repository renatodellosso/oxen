#include "../../src/compiler/expression.hpp"
#include <algorithm>
#include <gtest/gtest.h>

static std::vector<Expression *>
completionPointers(const Expression &expression) {
  std::vector<Expression *> pointers;
  for (auto completion : expression.getCompletionExpressions())
    pointers.push_back(&completion.get());
  return pointers;
}

TEST(ExprDependent, CustomHasherIsConsistentWithoutArgIndices) {
  Expression expr(InstructionType::Block, 0);
  ExprDependent dep(expr);

  std::hash<ExprDependent> hasher;
  auto origHash = hasher(dep);

  for (int i = 0; i < 100; i++)
    ASSERT_EQ(hasher(dep), origHash);
}

TEST(ExprDependent, CustomHasherIsConsistentWithArgIndices) {
  Expression expr(InstructionType::Block, 0);
  ExprDependent dep(expr, 1);

  std::hash<ExprDependent> hasher;
  auto origHash = hasher(dep);

  for (int i = 0; i < 100; i++)
    ASSERT_EQ(hasher(dep), origHash);
}

TEST(ExprDependent, preventsDuplicatesInUnorderedSets) {
  Expression expr(InstructionType::Block, 0);
  ExprDependent dep(expr, 1);

  std::unordered_set<ExprDependent> set;

  set.insert(dep);
  set.insert(dep);

  EXPECT_EQ(set.size(), 1);
}

TEST(ExprDependent, allowsDifferentInUnorderedSets) {
  Expression expr1(InstructionType::Block, 0);
  Expression expr2(InstructionType::Block, 1);
  ExprDependent dep1(expr1, 1);
  ExprDependent dep2(expr1, 2);
  ExprDependent dep3(expr1);
  ExprDependent dep4(expr2);

  std::unordered_set<ExprDependent> set;

  set.insert(dep1);
  set.insert(dep2);
  set.insert(dep3);
  set.insert(dep4);

  EXPECT_EQ(set.size(), 4);
}

TEST(ExpressionCompletion, ordinaryExpressionCompletesWithItself) {
  Expression expression(InstructionType::Print, 1);

  EXPECT_EQ(completionPointers(expression),
            std::vector<Expression *>{&expression});
}

TEST(ExpressionCompletion, emptyBlockCompletesWithTheBlockInstruction) {
  BlockExpression block(1);

  EXPECT_EQ(completionPointers(block), std::vector<Expression *>{&block});
}

TEST(ExpressionCompletion, blockCombinesNestedStatementCompletions) {
  auto first = std::make_shared<Expression>(InstructionType::Print, 1);
  auto nested = std::make_shared<Expression>(InstructionType::Set, 1);
  auto nestedBlock = std::make_shared<BlockExpression>(
      std::vector<std::shared_ptr<Expression>>{nested}, 1);
  BlockExpression block(
      std::vector<std::shared_ptr<Expression>>{first, nestedBlock}, 1);

  EXPECT_EQ(completionPointers(block),
            (std::vector<Expression *>{first.get(), nested.get()}));
}

TEST(ExpressionCompletion, ifExpressionCompletesWithItsMerge) {
  auto condition = std::make_shared<RootExpression>(
      InstructionType::GetLiteral, 1,
      Token(TokenType::Literal, TokenSubtype::Bool, "true", 1));
  auto thenBlock = std::make_shared<BlockExpression>(1);
  IfExpression ifExpression(1, condition, thenBlock);

  EXPECT_EQ(completionPointers(ifExpression),
            std::vector<Expression *>{ifExpression.mergeInstruction.get()});
}

TEST(ExpressionCompletion, markedBlockUsesItsExplicitCompletion) {
  auto condition = std::make_shared<RootExpression>(
      InstructionType::GetLiteral, 1,
      Token(TokenType::Literal, TokenSubtype::Bool, "true", 1));
  auto whileInstruction = std::make_shared<UnaryExpression>(
      InstructionType::While, 1, condition);
  auto bodyInstruction =
      std::make_shared<Expression>(InstructionType::Set, 1);
  BlockExpression loopBlock(
      std::vector<std::shared_ptr<Expression>>{whileInstruction,
                                                bodyInstruction},
      1);
  loopBlock.completionExpression = whileInstruction;

  EXPECT_EQ(completionPointers(loopBlock),
            std::vector<Expression *>{whileInstruction.get()});
}

TEST(IfExpression, mergeDoesNotDependOnRepeatableElseLoopInstructions) {
  auto ifCondition = std::make_shared<RootExpression>(
      InstructionType::GetLiteral, 1,
      Token(TokenType::Literal, TokenSubtype::Bool, "false", 1));
  auto thenBlock = std::make_shared<BlockExpression>(1);

  auto loopCondition = std::make_shared<RootExpression>(
      InstructionType::GetLiteral, 1,
      Token(TokenType::Literal, TokenSubtype::Bool, "true", 1));
  auto whileInstruction = std::make_shared<UnaryExpression>(
      InstructionType::While, 1, loopCondition);
  auto bodyInstruction = std::make_shared<RootExpression>(
      InstructionType::GetLiteral, 1,
      Token(TokenType::Literal, TokenSubtype::Integer, "1", 1));
  auto goToInstruction = std::make_shared<RootExpression>(
      InstructionType::GoTo, 1,
      Token(TokenType::Literal, TokenSubtype::Integer, "-4", 1));
  auto loopBody = std::make_shared<BlockExpression>(
      std::vector<std::shared_ptr<Expression>>{bodyInstruction,
                                                goToInstruction},
      1);
  auto loweredLoop = std::make_shared<BlockExpression>(
      std::vector<std::shared_ptr<Expression>>{whileInstruction, loopBody}, 1);
  loweredLoop->completionExpression = whileInstruction;
  auto elseBlock = std::make_shared<BlockExpression>(
      std::vector<std::shared_ptr<Expression>>{loweredLoop}, 1);

  IfExpression ifExpression(1, ifCondition, thenBlock, elseBlock);
  ifExpression.linkInternally();

  auto mergeDependsOn = [&ifExpression](const Expression &dependency) {
    return std::ranges::any_of(
        ifExpression.mergeInstruction->dependencies,
        [&dependency](const auto &candidate) {
          return &candidate.get() == &dependency;
        });
  };

  // The While instruction publishes once, when its condition becomes false,
  // so it is the stable completion edge for the whole loop.
  EXPECT_TRUE(mergeDependsOn(*whileInstruction));

  // These instructions publish again on every iteration. Direct merge edges
  // let repeated signals over-fulfil the merge's fixed dependency count.
  EXPECT_FALSE(mergeDependsOn(*loopCondition));
  EXPECT_FALSE(mergeDependsOn(*loweredLoop));
  EXPECT_FALSE(mergeDependsOn(*loopBody));
  EXPECT_FALSE(mergeDependsOn(*bodyInstruction));
  EXPECT_FALSE(mergeDependsOn(*goToInstruction));
}
