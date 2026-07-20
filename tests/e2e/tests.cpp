#include "tests.hpp"

std::vector<E2eTest> tests = {
    // Print statements
    {"PrintWorksWithNumbers", "print 1;", ExpectUnordered({"1"})},
    {"PrintWorksWithStrings", "print \"abc\";", ExpectUnordered({"abc"})},
    {"PrintWorksWithBools", "print true;", ExpectUnordered({"true"})},
    {"PrintWorksWithEmptyStrings", "print \"\";", ExpectUnordered({""})},
    {"PrintPreservesStringWhitespace", "print \"a b\t c\";",
     ExpectUnordered({"a b\t c"})},
    {"PrintWorksWithMultiplePrints", "print 2;\nprint false;",
     ExpectUnordered({"false", "2"})},

    // Addition
    {"AdditionWorksWithNumbers", "print 1 + 2;", ExpectUnordered({"3"})},
    {"AdditionWorksWithThreeNumbers", "print 1 + 1 + 1;",
     ExpectUnordered({"3"})},
    {"AdditionWorksWithStrings", "print \"a\" + \"b\";",
     ExpectUnordered({"ab"})},
    {"AdditionWorksWithBools", "print false + true;",
     ExpectUnordered({"true"})},
    {"AdditionWorksWithFalseBools", "print false + false;",
     ExpectUnordered({"false"})},
    {"AdditionWorksWithMixedTypes", "print 1 + \"b\" + true;",
     ExpectUnordered({"1btrue"})},
    {"AdditionStringifiesMixedTypesInEitherOperandOrder",
     "print true + \"b\" + 1;", ExpectUnordered({"trueb1"})},
    {"MixedArithmeticOperatorsAreGroupedFromTheRight",
     "print 2 * 3 + 4;\nprint 1 + 2 * 3;", ExpectUnordered({"14", "7"})},

    // Subtraction
    {"SubtractionWorks", "print 1 - 1;", ExpectUnordered({"0"})},
    {"SubtractionWorksWithThreeNumbers", "print 1 - 1 - 2;",
     ExpectUnordered({"2"})},
    {"SubtractionWorksWithNegativeNumbers", "print 1 - -1;",
     ExpectUnordered({"2"})},

    // Multiplication
    {"MultiplicationWorks", "print 2 * 2;", ExpectUnordered({"4"})},
    {"MultiplicationWithThreeNumbers", "print 2 * 3 * 2;",
     ExpectUnordered({"12"})},
    {"MultiplicationWorksWithNegativeNumbers", "print 1 * -1;",
     ExpectUnordered({"-1"})},

    // Division
    {"DivisionWorksWhenCleanlyDivisible", "print 4 / 2;",
     ExpectUnordered({"2"})},
    {"DivisionWorksWhenNotCleanlyDivisible", "print 3 / 2;",
     ExpectUnordered({"1"})},
    {"DivisionWorksWithNegativeNumbers", "print 1 / -1;",
     ExpectUnordered({"-1"})},
    {"DivisionWorksWithNegativeDividends", "print -3 / 2;",
     ExpectUnordered({"-1"})},
    {"DivisionChainsAreGroupedFromTheRight", "print 8 / 4 / 2;",
     ExpectUnordered({"4"})},
    {"DivisionByZeroReportsRuntimeError", "print 1 / 0;",
     ExpectError(ExitCode::ExecutionError, "dIvIsIoN bY zErO")},

    // Equality
    {"EqualityWorksWithEqualNumbers", "print 1 == 1;",
     ExpectUnordered({"true"})},
    {"EqualityWorksWithUnequalNumbers", "print 1 == 2;",
     ExpectUnordered({"false"})},
    {"EqualityWorksWithEqualStrings", "print \"a\" == \"a\";",
     ExpectUnordered({"true"})},
    {"EqualityWorksWithUnequalStrings", "print \"a\" == \"b\";",
     ExpectUnordered({"false"})},
    {"EqualityWorksWithEqualBools", "print true == true;",
     ExpectUnordered({"true"})},
    {"EqualityWorksWithUnequalBools", "print true == false;",
     ExpectUnordered({"false"})},
    {"EqualityDefaultsToFalseForMixedTypes", "print 1 == true;",
     ExpectUnordered({"false"})},
    {"EqualityWorksInConditions",
     "int count = 0;\n"
     "if (count == 0) print \"done\";",
     ExpectUnordered({"done"})},

    // Inequality
    {"InequalityWorksWithUnequalNumbers", "print 1 != 2;",
     ExpectUnordered({"true"})},
    {"InequalityWorksWithEqualNumbers", "print 1 != 1;",
     ExpectUnordered({"false"})},
    {"InequalityWorksWithUnequalStrings", "print \"a\" != \"b\";",
     ExpectUnordered({"true"})},
    {"InequalityWorksWithEqualStrings", "print \"a\" != \"a\";",
     ExpectUnordered({"false"})},
    {"InequalityWorksWithBools", "print true != false;",
     ExpectUnordered({"true"})},
    {"InequalityDefaultsToTrueForMixedTypes", "print 1 != true;",
     ExpectUnordered({"true"})},

    // Ordered comparisons
    {"LessThanWorksWithNumbers", "print 1 < 2;", ExpectUnordered({"true"})},
    {"LessThanWorksWithNumbersFalse", "print 2 < 1;",
     ExpectUnordered({"false"})},
    {"LessThanEqualsWorksWithNumbers", "print 2 <= 2;",
     ExpectUnordered({"true"})},
    {"GreaterThanWorksWithNumbers", "print 2 > 1;", ExpectUnordered({"true"})},
    {"GreaterThanEqualsWorksWithNumbers", "print 2 >= 2;",
     ExpectUnordered({"true"})},
    {"LessThanComparesStringsAlphabetically", "print \"a\" < \"b\";",
     ExpectUnordered({"true"})},
    {"GreaterThanComparesStringsAlphabetically", "print \"b\" > \"a\";",
     ExpectUnordered({"true"})},
    {"LessThanEqualsComparesStringsAlphabetically", "print \"a\" <= \"a\";",
     ExpectUnordered({"true"})},
    {"GreaterThanEqualsComparesStringsAlphabetically", "print \"b\" >= \"a\";",
     ExpectUnordered({"true"})},
    {"OrderedComparisonsWorkInConditions",
     "if (\"a\" < \"b\") print \"ordered\";", ExpectUnordered({"ordered"})},

    // Variables
    {"VariablesCanBeDeclared", "int a;", ExpectUnordered({})},
    {"VariablesCanBeDeclaredAndInitialized", "int a = 1;\nprint a;",
     ExpectUnordered({"1"})},
    {"VariablesCanBeSet", "int a;\na = 1;\nprint a;", ExpectUnordered({"1"})},
    {"VariablesCanBeSetMultipleTimes", "int a;\na = 1;\na = 2;\nprint a;",
     ExpectUnordered({"2"})},
    {"VariablesCanBeUsedInOperations", "int a = 1;\nprint a + 1;",
     ExpectUnordered({"2"})},
    {"VariablesCanBeDeclaredMultipleTimes",
     "int a = 1;\nprint a;\nint b = 2;\nprint b;", ExpectUnordered({"1", "2"})},
    {"VariablesCanHaveBoolType", "bool a = true;\nprint a;",
     ExpectUnordered({"true"})},
    {"VariablesCanHaveStringType", "string a = \"abc\";\nprint a;",
     ExpectUnordered({"abc"})},
    {"VariablesCanBeUsedToUpdateThemselves", "int a = 1;\na = a + 1;\nprint a;",
     ExpectUnordered({"2"})},
    {"VariableNamesCanContainDigits", "int value2 = 2;\nprint value2;",
     ExpectUnordered({"2"})},
    {"NestedBlocksCanReadAndUpdateEnclosingVariables",
     "int a = 1;\n{ a = a + 1; }\nprint a;", ExpectUnordered({"2"})},
    {"NestedBlocksCanShadowEnclosingVariables",
     "int a = 1;\n{ int a = 2; print a; }\nprint a;",
     ExpectUnordered({"1", "2"})},
    {"EmptyBlocksAreAllowed", "{}\nprint \"done\";", ExpectUnordered({"done"})},

    // If statements
    {"IfsDontRunIfConditionIsFalse", "if (false) { print \"ran\"; }",
     ExpectUnordered({})},
    {"IfsRunIfConditionIsTrue", "if (true) { print \"ran\"; }",
     ExpectUnordered({"ran"})},
    {"IfsAllowImplicitBlocks", "if (true) print \"ran\";",
     ExpectUnordered({"ran"})},
    {"IfsAllowComplexConditions", "if (1 + 1 - 1) print \"ran\";",
     ExpectUnordered({"ran"})},
    {"IfsAllowVariablesInCondition", "bool a = true;\nif (a) print \"ran\";",
     ExpectUnordered({"ran"})},
    {"IfsTreatZeroAsFalse", "if (0) print \"ran\";", ExpectUnordered({})},
    {"IfsTreatNegativeIntegersAsTrue", "if (-1) print \"ran\";",
     ExpectUnordered({"ran"})},
    {"IfsTreatEmptyStringsAsFalse", "if (\"\") print \"ran\";",
     ExpectUnordered({})},
    {"IfsTreatNonEmptyStringsAsTrue", "if (\"a\") print \"ran\";",
     ExpectUnordered({"ran"})},
    {"ElsesDoNotRunIfConditionIsTrue",
     "if (true) print \"then\"; else print \"else\";",
     ExpectUnordered({"then"})},
    {"ElsesRunIfConditionIsFalse",
     "if (false) print \"then\"; else print \"else\";",
     ExpectUnordered({"else"})},
    {"ElsesAllowBlocks",
     "if (false) { print \"then\"; } else { print \"else\"; }",
     ExpectUnordered({"else"})},
    {"ElseIfsRunFirstTrueBranch",
     "if (false) print \"if\";\n"
     "else if (true) print \"else if\";\n"
     "else print \"else\";",
     ExpectUnordered({"else if"})},
    {"ElseIfsRunFinalElse",
     "if (false) print \"if\";\n"
     "else if (false) print \"else if\";\n"
     "else print \"else\";",
     ExpectUnordered({"else"})},
    {"ElsesSetVariablesFromThenBranch",
     "int a = 0;\n"
     "if (true) a = 1; else a = 2;\n"
     "print a;",
     ExpectUnordered({"1"})},
    {"ElsesSetVariablesFromElseBranch",
     "int a = 0;\n"
     "if (false) a = 1; else a = 2;\n"
     "print a;",
     ExpectUnordered({"2"})},
    {"IfBranchWritesAreVisibleAfterTrueBranch",
     "int a = 0;\nif (true) a = 1;\nprint a;", ExpectUnordered({"1"})},
    {"SkippedIfBranchPreservesPreviousValue",
     "int a = 0;\nif (false) a = 1;\nprint a;", ExpectUnordered({"0"})},
    {"IfsCanBeNested",
     "if (true) { if (false) print \"wrong\"; else print \"nested\"; }",
     ExpectUnordered({"nested"})},
    {"ElseBlocksCanContainLoops",
     "int i = 0;\n"
     "if (false) print \"wrong\"; else { while (i < 2) i = i + 1; }\n"
     "print i;",
     ExpectUnordered({"2"})},

    // While loops
    {"WhileLoopsRunWhileConditionIsTrue",
     "int count = 10;\n"
     "while (count) {\n"
     "print count;\n"
     "count = count - 1;\n"
     "}",
     ExpectUnordered({"10", "9", "8", "7", "6", "5", "4", "3", "2", "1"})},
    {"WhileLoopsAllowImplicitBlocks",
     "int count = 10;\n"
     "while (count)\n"
     "count = count - 1;\n"
     "print count;",
     ExpectUnordered({"0"})},
    {"WhileLoopsCanBeInsideIfStatements",
     "if (true) {\n"
     "int a = 5;\n"
     "while (a) {\n"
     "print a;\n"
     "a = a - 1;\n"
     "}\n"
     "}",
     ExpectUnordered({"1", "2", "3", "4", "5"})},
    {"WhileLoopsUpdateConditionAcrossIterations",
     "int i = 0;\n"
     "while (i < 3) {\n"
     "i = i + 1;\n"
     "}\n"
     "print i;",
     ExpectUnordered({"3"})},
    {"WhileLoopsCanRunZeroIterations",
     "int i = 0;\nwhile (false) i = i + 1;\nprint i;", ExpectUnordered({"0"})},
    {"WhileLoopsCanUseStringTruthiness",
     "string value = \"run\";\n"
     "while (value) { print value; value = \"\"; }",
     ExpectUnordered({"run"})},
    {"WhileLoopsCanContainIfStatements",
     "int i = 0;\n"
     "while (i < 3) {\n"
     "if (i == 1) print \"middle\";\n"
     "i = i + 1;\n"
     "}",
     ExpectUnordered({"middle"})},
    {
        "WhileLoopsCanCountIterationsCorrectly",
        "int sum = 0;\n"
        "int i = 0;\n"
        "while (i < 3) {\n"
        "sum = sum + i;\n"
        "i = i + 1;\n"
        "}\n"
        "print sum;",
        ExpectUnordered({"3"}),
    },
    {
        "WhileLoopsCanBeNestedAndCountIterationsCorrectly",
        "int sum = 0;\n"
        "int i = 0;\n"
        "while (i < 3) {\n"
        "int j = 0;\n"
        "while (j < 2) {\n"
        "sum = sum + i + j;\n"
        "j = j + 1;\n"
        "}\n"
        "i = i + 1;\n"
        "}\n"
        "print sum;",
        ExpectUnordered({"9"}),
    },
    {
        "NestedLoopsRunInnerLoopForEachOuterIteration",
        "int count = 0;\n"
        "int i = 0;\n"
        "while (i < 3) {\n"
        "int j = 0;\n"
        "while (j < 2) {\n"
        "count = count + 1;\n"
        "j = j + 1;\n"
        "}\n"
        "i = i + 1;\n"
        "}\n"
        "print count;",
        ExpectUnordered({"6"}),
    },
    {"WhileLoopsCanBeDoublyNestedAndCountIterationsCorrectly",
     "int sum = 0;\n"
     "int i = 0;\n"
     "while (i < 3) {\n"
     "int j = 0;\n"
     "while (j < 2) {\n"
     "int k = 0;\n"
     "while (k < 2) {\n"
     "sum = sum + i + j + k;\n"
     "k = k + 1;\n"
     "}\n"
     "j = j + 1;\n"
     "}\n"
     "i = i + 1;\n"
     "}\n"
     "print sum;",
     ExpectUnordered({"24"})},
    {"WhileLoopsCanBeNestedWhenTheyDontUseExternalVars",
     "int i = 0;\n"
     "while (i < 3) {\n"
     "int j = 0;\n"
     "while (j < 2) {\n"
     "print \"j: \" + j;\n"
     "j = j + 1;\n"
     "}\n"
     "i = i + 1;\n"
     "}\n",
     ExpectUnordered({"j: 0", "j: 1", "j: 0", "j: 1", "j: 0", "j: 1"})},
    {"WhileLoopsCanBeDoublyNestedWhenTheyDontUseExternalVars",
     "int i = 0;\n"
     "while (i < 3) {\n"
     "int j = 0;\n"
     "while (j < 2) {\n"
     "int k = 0;\n"
     "while (k < 2) {\n"
     "print \"k: \" + k;\n"
     "k = k + 1;\n"
     "}\n"
     "j = j + 1;\n"
     "}\n"
     "i = i + 1;\n"
     "}\n",
     ExpectUnordered({"k: 0", "k: 1", "k: 0", "k: 1", "k: 0", "k: 1", "k: 0",
                      "k: 1", "k: 0", "k: 1", "k: 0", "k: 1"})},
    {"LoopBackWaitsForDependencyPublication",
     "int i = 0;\n"
     "while (i < 10) {\n"
     "int j = 0;\n"
     "while (j < 5) {\n"
     "int k = 0;\n"
     "while (k < 3) {\n"
     "print \"k: \" + k;\n"
     "k = k + 1;\n"
     "}\n"
     "j = j + 1;\n"
     "}\n"
     "i = i + 1;\n"
     "}\n",
     ExpectUnordered([] {
       std::vector<std::string> output;
       for (int i = 0; i < 10; i++)
         for (int j = 0; j < 5; j++)
           for (int k = 0; k < 3; k++)
             output.push_back("k: " + std::to_string(k));
       return output;
     }())},
    {"WhileLoopsCanBeNestedWhenTheyUseExternalVars",
     "int i = 0;\n"
     "while (i < 3) {\n"
     "int j = 0;\n"
     "while (j < 2) {\n"
     "print i + \" \" + j;\n"
     "j = j + 1;\n"
     "}\n"
     "i = i + 1;\n"
     "}\n",
     ExpectUnordered({"0 0", "0 1", "1 0", "1 1", "2 0", "2 1"})},
    {"WhileLoopsCanBeDoublyNestedWhenTheyUseExternalVars",
     "int i = 0;\n"
     "while (i < 3) {\n"
     "int j = 0;\n"
     "while (j < 2) {\n"
     "int k = 0;\n"
     "while (k < 2) {\n"
     "print i + \" \" + j + \" \" + k;\n"
     "k = k + 1;\n"
     "}\n"
     "j = j + 1;\n"
     "}\n"
     "i = i + 1;\n"
     "}\n",
     ExpectUnordered({"0 0 0", "0 0 1", "0 1 0", "0 1 1", "1 0 0", "1 0 1",
                      "1 1 0", "1 1 1", "2 0 0", "2 0 1", "2 1 0", "2 1 1"})},

    // Functions
    {"FunctionsCanBeDeclared", "void main() { print 1; }", ExpectUnordered({})},
    {"FunctionsCanBeDeclaredWithParameters",
     "void main(int a, string b) { print 1; }", ExpectUnordered({})},
    {"FunctionsCanBeDeclaredWithReturnType", "int main() { print 1; }",
     ExpectUnordered({})},
    {"FunctionsCanBeDeclaredWithReturnTypeAndParameters",
     "int main(int a, string b) { print 1; }", ExpectUnordered({})},
    {"FunctionsCanBeDeclaredWhileUsingParametersInBodies",
     "int main(int a, string b) { print a + b; }", ExpectUnordered({})},
    {"FunctionsCanBeDeclaredMultipleTimes",
     "void main() { print 1; }\n"
     "void extra() { print 2; }",
     ExpectUnordered({})},
    {"FunctionsCanBeDeclaredInsideFunctions",
     "void main() { \n"
     "void extra() { print 2; }\n"
     "}",
     ExpectUnordered({})},
    {"FunctionsCanBeDeclaredInsideMultipleFunctions",
     "void main() { \n"
     "void extra() { \n"
     "void extra2() { print 2; }\n"
     " }\n"
     "}",
     ExpectUnordered({})},
    {"FunctionsCanShadowVariablesWithParameters",
     "int a;\n"
     "void main(int a) { \n"
     "print a;"
     "}",
     ExpectUnordered({})},
    {"FunctionParametersShadowVariablesWhenCalled",
     "int a = 1;\n"
     "void printValue(int a) { print a; }\n"
     "printValue(2);\n"
     "print a;",
     ExpectUnordered({"1", "2"})},

    // Calls
    {"CallsCallFunctions",
     "void main() { \n"
     "print \"Func!\";\n"
     "}\n"
     "main();",
     ExpectUnordered({"Func!"})},
    {"CallsCanCallFunctionsMultipleTimes",
     "void main() { \n"
     "print \"Func!\";\n"
     "}\n"
     "main();\n"
     "main();",
     ExpectUnordered({"Func!", "Func!"})},
    {"CallsCanCallDifferentFunctions",
     "void a() { \n"
     "print \"A\";\n"
     "}\n"
     "void b() { \n"
     "print \"B\";\n"
     "}\n"
     "a();\n"
     "b();",
     ExpectUnordered({"A", "B"})},
    {"CallsCanCallFunctionsWithIfStatementsInside",
     "void main() { \n"
     "if (true)\n"
     "print \"A\";\n"
     "if (false)\n"
     "print \"B\";\n"
     "}\n"
     "main();",
     ExpectUnordered({"A"})},
    {"CallsCanCallFunctionsWithWhileStatementsInside",
     "void main() { \n"
     "int a = 5;\n"
     "while (a) {\n"
     "print a;\n"
     "a = a - 1;\n"
     "}\n"
     "}\n"
     "main();",
     ExpectUnordered({"5", "4", "3", "2", "1"})},
    {"CallsCanCallFunctionsRecursively",
     "bool recurse = true;\n"
     "void main() {\n"
     "print recurse;\n"
     "if (recurse) {\n"
     "recurse = false;\n"
     "main();\n"
     "}"
     "}\n"
     "main();",
     ExpectUnordered({"true", "false"})},
    {"CallsAreSequencedCorrectly",
     "int a = 0;\n"
     "void main() {\n"
     "print a;\n"
     "}\n"
     "main();\n"
     "a = a + 1;\n"
     "main();\n"
     "a = a + 1;\n"
     "main();\n",
     ExpectOrdered({"0", "1", "2"})},
    {"CallsAreSequencedCorrectlyWhenWritesAreInCalls",
     "int a = 0;\n"
     "void main() {\n"
     "a = 1;\n"
     "}\n"
     "main();\n"
     "print a;\n",
     ExpectUnordered({"1"})},
    {"CallsWorkWithArguments",
     "void add(int a, int b) {\n"
     "print a + b;\n"
     "}\n"
     "add(1, 2);",
     ExpectUnordered({"3"})},
    {"CallsAcceptExpressionArguments",
     "void printArg(int a) { print a; }\n"
     "printArg(1 + 2);",
     ExpectUnordered({"3"})},
    {"CallsWithArgumentsDoNotShareParameterScopes",
     "void printArg(int a) {\n"
     "print a;\n"
     "}\n"
     "printArg(1);\n"
     "printArg(2);\n"
     "printArg(3);",
     ExpectUnordered({"1", "2", "3"})},
    {"CallsWorkFromEnclosedFunctionsToEnclosingFunctions",
     "void outer(int x) {\n"
     "void inner(bool y) {\n"
     "outer(0);\n"
     "}\n"
     "print x - 1;\n"
     "if (x)\n"
     "inner(true);\n"
     "print x + 1;\n"
     "}\n"
     "outer(5);\n",
     ExpectUnordered({"-1", "1", "4", "6"})},
    {"CallsCanCallFunctionsInsideFunctions",
     "void main() { \n"
     "void extra() { print 2; }\n"
     "extra();\n"
     "}\n"
     "main();",
     ExpectUnordered({"2"})},
    {"CallsCanCallFunctionsInsideFunctionsWithArguments",
     "void main(int a) { \n"
     "void extra(int b) { print a + b; }\n"
     "extra(2);\n"
     "}\n"
     "main(3);",
     ExpectUnordered({"5"})},
    {"CallsCanCallFunctionsInsideMultipleFunctions",
     "void main() { \n"
     "void extra() { \n"
     "void extra2() { print 2; }\n"
     "extra2();\n"
     "}\n"
     "extra();\n"
     "}\n"
     "main();",
     ExpectUnordered({"2"})},
    {"CallsCanCallFunctionsInsideMultipleFunctionsWithArguments",
     "void main(int a) { \n"
     "void extra(int b) { \n"
     "void extra2(int c) { print a + b + c; }\n"
     "extra2(3);\n"
     "}\n"
     "extra(2);\n"
     "}\n"
     "main(1);",
     ExpectUnordered({"6"})},
    {"CallsCanBeNestedAsArguments",
     "int increment(int a) { return a + 1; }\n"
     "int twice(int a) { return a * 2; }\n"
     "print twice(increment(2));",
     ExpectUnordered({"6"})},
    {"CallResultsCanBeAssignedAndUsedInArithmetic",
     "int increment(int a) { return a + 1; }\n"
     "int value = increment(2);\n"
     "print value + increment(3);",
     ExpectUnordered({"7"})},
    {"CallResultsCanBeUsedInComparisonsAndConditions",
     "int identity(int a) { return a; }\n"
     "if (identity(2) == 2) print \"equal\";\n"
     "if (identity(1)) print \"truthy\";",
     ExpectUnordered({"equal", "truthy"})},
    {"FunctionsCanBeCalledInsideLoops",
     "void printArg(int a) { print a; }\n"
     "int i = 0;\n"
     "while (i < 3) { printArg(i); i = i + 1; }",
     ExpectOrdered({"0", "1", "2"})},

    {"ReturnsWork", "int main() { return 1; }\nprint main();",
     ExpectUnordered({"1"})},
    {"ReturnsWorkWithMultipleCalls",
     "int main(bool a) {\n"
     "if (a) return 1;\n"
     "else return 2;\n"
     "}\n"
     "print main(true);\n"
     "print main(false);",
     ExpectUnordered({"1", "2"})},
    {"ReturnsWorkWithEnclosedFunctions",
     "void outer() {\n"
     "void inner() {\n"
     "return 1;\n"
     "}\n"
     "print inner();\n"
     "print \"done\";\n"
     "}\n"
     "outer();",
     ExpectUnordered({"1", "done"})},
    {"ReturnsWorkWithMultipleReturnStatementsInOneFunction",
     "int main(int a) {\n"
     "if (a == 0) return 1;\n"
     "if (a == 1) return 2;\n"
     "}\n"
     "print main(0);\n"
     "print main(1);\n",
     ExpectUnordered({"1", "2"})},
    {"ReturnsUseFirstExecutedReturnStatement",
     "int main() {\n"
     "return 1;\n"
     "print \"done\";\n"
     "return 2;\n"
     "}\n"
     "print main();",
     ExpectUnordered({"1", "done"})},
    {"ReturnsRunLaterSideEffectsWithoutOverwritingReturnValue",
     "int a = 0;\n"
     "int main() {\n"
     "return 1;\n"
     "a = 5;\n"
     "return 2;\n"
     "}\n"
     "print main();\n"
     "print a;",
     ExpectUnordered({"1", "5"})},
    {"ReturnsDoNotShareStateAcrossRepeatedArgumentCalls",
     "int id(int a) {\n"
     "return a;\n"
     "}\n"
     "print id(10);\n"
     "print id(20);\n"
     "print id(30);",
     ExpectUnordered({"10", "20", "30"})},
    {"ReturnsDoNotEndFunctionsPrematurely",
     "int main() {\n"
     "return 0;\n"
     "print \"done\";\n"
     "}\n"
     "print main();",
     ExpectUnordered({"0", "done"})},

    // Invalid programs
    {"UndefinedVariablesReportSyntaxErrors", "print missing;",
     ExpectError(ExitCode::SyntaxErrors, "does not exist")},
    {"UndefinedFunctionsReportSyntaxErrors", "missing();",
     ExpectError(ExitCode::SyntaxErrors, "did not exist")},
    {"DuplicateDeclarationsReportSyntaxErrors", "int a;\nint a;",
     ExpectError(ExitCode::SyntaxErrors, "already existed")},
    {"SubtractionRejectsNonIntegerOperands", "print true - false;",
     ExpectError(ExitCode::ExecutionError, "Invalid arg types")},
    {"MultiplicationRejectsNonIntegerOperands", "print \"a\" * 2;",
     ExpectError(ExitCode::ExecutionError, "Invalid arg types")},
    {"DivisionRejectsNonIntegerOperands", "print true / 2;",
     ExpectError(ExitCode::ExecutionError, "Invalid arg types")},
    {"OrderedComparisonsRejectBoolOperands", "print false < true;",
     ExpectError(ExitCode::ExecutionError, "Invalid arg types")},
    {"OrderedComparisonsRejectMixedOperands", "print 1 < \"1\";",
     ExpectError(ExitCode::ExecutionError, "Invalid arg types")},
    {"MissingClosingBracesReportSyntaxErrors", "if (true) { print 1;",
     ExpectError(ExitCode::SyntaxErrors, "Expected '}'")},
    {"StrayElseReportsSyntaxErrors", "else print 1;",
     ExpectError(ExitCode::SyntaxErrors, "without an if")},

};
