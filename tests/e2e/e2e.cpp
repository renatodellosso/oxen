#include "../../src/cli.hpp"
#include "../testUtils.hpp"
#include "tests.hpp"
#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>
#include <string>

// Int is thread count
class E2EFixture : public testing::TestWithParam<std::tuple<E2eTest, int>> {};

const std::string folder = "temp";

std::string getTestName(const E2eTest &test, int threads) {
  return test.name + std::to_string(threads);
}

TEST_P(E2EFixture, E2E) {
  const auto &test = std::get<E2eTest>(GetParam());
  auto threads = std::get<int>(GetParam());

  // Ensure folder exists
  std::filesystem::create_directory(folder);
  auto fileName = folder + "/" + getTestName(test, threads) + ".p";

  CliArgs args = {.target = fileName,
                  .mode = CliMode::CompileAndInterpret,
                  .threads = threads};

  // Create temp file with code
  std::ofstream fileStream(fileName);
  fileStream << test.code;
  fileStream.close();

  DISABLE_COUT
  auto exitCode = executeCommand(args);
  auto output = REENABLE_COUT;

  std::filesystem::remove(fileName);

  test.validate(exitCode, output);
}

INSTANTIATE_TEST_SUITE_P(
    E2E, E2EFixture,
    testing::Combine(testing::ValuesIn(tests.begin(), tests.end()),
                     testing::Values(1, 2, 4, 8, 16)),
    [](const testing::TestParamInfo<E2EFixture::ParamType> &info) {
      return getTestName(std::get<E2eTest>(info.param),
                         std::get<int>(info.param));
    });
