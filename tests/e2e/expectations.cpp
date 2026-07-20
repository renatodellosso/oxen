#include "tests.hpp"
#include "gmock/gmock.h"
#include <algorithm>
#include <cctype>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <memory>
#include <sstream>
#include <utility>

namespace {
std::vector<std::string> splitLines(const std::string &output) {
  std::vector<std::string> lines;
  std::stringstream stream(output);
  std::string line;

  while (std::getline(stream, line))
    lines.push_back(line);

  return lines;
}

std::string lowercase(std::string value) {
  std::ranges::transform(value, value.begin(), [](unsigned char character) {
    return static_cast<char>(std::tolower(character));
  });
  return value;
}
} // namespace

void ExpectOutput::validate(ExitCode exitCode,
                            const std::string &output) const {
  ASSERT_EQ(exitCode, ExitCode::Ok);
  validateLines(splitLines(output));
}

void ExpectOrdered::validateLines(
    const std::vector<std::string> &actual) const {
  EXPECT_THAT(actual, testing::ElementsAreArray(lines));
}

void ExpectUnordered::validateLines(
    const std::vector<std::string> &actual) const {
  EXPECT_THAT(actual, testing::UnorderedElementsAreArray(lines));
}

void ExpectError::validate(ExitCode actualExitCode,
                           const std::string &output) const {
  ASSERT_NE(exitCode, ExitCode::Ok)
      << "ExpectError must specify a non-OK exit code";
  ASSERT_EQ(actualExitCode, exitCode);
  EXPECT_THAT(lowercase(output), testing::HasSubstr(lowercase(messageSubstring)));
}

E2eTest::E2eTest(std::string name, std::string code, ExpectOrdered expectation)
    : name(std::move(name)), code(std::move(code)),
      expectation(std::make_shared<ExpectOrdered>(std::move(expectation))) {}

E2eTest::E2eTest(std::string name, std::string code,
                 ExpectUnordered expectation)
    : name(std::move(name)), code(std::move(code)),
      expectation(std::make_shared<ExpectUnordered>(std::move(expectation))) {}

E2eTest::E2eTest(std::string name, std::string code, ExpectError expectation)
    : name(std::move(name)), code(std::move(code)),
      expectation(std::make_shared<ExpectError>(std::move(expectation))) {}

void E2eTest::validate(ExitCode exitCode, const std::string &output) const {
  expectation->validate(exitCode, output);
}
