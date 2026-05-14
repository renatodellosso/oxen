#include "../../src/compiler/deferredFunctionLinking.hpp"
#include "../../src/interpreter/subprogram.hpp"
#include <functional>
#include <gtest/gtest.h>
#include <memory>
#include <unordered_set>

TEST(DeferredFunctionLinking, producesConsistentHashes) {
  Instruction instr(0);
  Subprogram program;

  FunctionExpression func;
  auto scope = std::make_shared<Scope<Resource>>();

  std::hash<std::reference_wrapper<FunctionExpression>> hasher;
  auto origHash = hasher(func);

  for (int i = 0; i < 100; i++)
    ASSERT_EQ(hasher(func), origHash);
}

TEST(DeferredFunctionLinking, preventsDuplicatesInUnorderedSets) {
  Instruction instr(0);
  Subprogram program;

  FunctionExpression func;
  auto scope = std::make_shared<Scope<Resource>>();

  std::unordered_set<std::reference_wrapper<FunctionExpression>> set;

  set.insert(func);
  set.insert(func);

  EXPECT_EQ(set.size(), 1);
}

TEST(DeferredFunctionLinking, allowsDifferentInUnorderedSets) {
  Instruction instr(0);
  Subprogram program;

  FunctionExpression func1;
  FunctionExpression func2;
  auto scope = std::make_shared<Scope<Resource>>();

  std::unordered_set<std::reference_wrapper<FunctionExpression>> set;

  set.insert(func1);
  set.insert(func2);

  EXPECT_EQ(set.size(), 2);
}