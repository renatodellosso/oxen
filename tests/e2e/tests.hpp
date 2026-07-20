#pragma once

#include "../../src/exitCode.hpp"
#include <initializer_list>
#include <memory>
#include <string>
#include <utility>
#include <vector>

class E2eExpectation {
public:
  virtual ~E2eExpectation() = default;
  virtual void validate(ExitCode exitCode, const std::string &output) const = 0;
};

class ExpectOutput : public E2eExpectation {
protected:
  std::vector<std::string> lines;

public:
  ExpectOutput(std::initializer_list<std::string> lines) : lines(lines) {}
  explicit ExpectOutput(std::vector<std::string> lines)
      : lines(std::move(lines)) {}

  void validate(ExitCode exitCode, const std::string &output) const final;

private:
  virtual void validateLines(const std::vector<std::string> &actual) const = 0;
};

class ExpectOrdered final : public ExpectOutput {
public:
  using ExpectOutput::ExpectOutput;

private:
  void validateLines(const std::vector<std::string> &actual) const override;
};

class ExpectUnordered final : public ExpectOutput {
public:
  using ExpectOutput::ExpectOutput;

private:
  void validateLines(const std::vector<std::string> &actual) const override;
};

class ExpectError final : public E2eExpectation {
  ExitCode exitCode;
  std::string messageSubstring;

public:
  ExpectError(ExitCode exitCode, std::string messageSubstring)
      : exitCode(exitCode), messageSubstring(std::move(messageSubstring)) {}

  void validate(ExitCode actualExitCode,
                const std::string &output) const override;
};

struct E2eTest {
  std::string name;
  std::string code;

  E2eTest(std::string name, std::string code, ExpectOrdered expectation);
  E2eTest(std::string name, std::string code, ExpectUnordered expectation);
  E2eTest(std::string name, std::string code, ExpectError expectation);

  void validate(ExitCode exitCode, const std::string &output) const;

private:
  std::shared_ptr<const E2eExpectation> expectation;
};

extern std::vector<E2eTest> tests;
