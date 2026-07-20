#include "../../benchmark/discovery.hpp"
#include <chrono>
#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>
#include <stdexcept>
#include <string>

namespace {

class TemporaryDirectory {
public:
  std::filesystem::path path;

  TemporaryDirectory() {
    auto suffix = std::chrono::steady_clock::now().time_since_epoch().count();
    path = std::filesystem::temp_directory_path() /
           ("parallel-benchmark-discovery-" + std::to_string(suffix));
    std::filesystem::create_directories(path);
  }

  ~TemporaryDirectory() { std::filesystem::remove_all(path); }
};

void touch(const std::filesystem::path &path) {
  std::ofstream file(path);
  file << "int value = 1;";
}

} // namespace

TEST(BenchmarkDiscovery, FindsProgramsInDeterministicOrder) {
  TemporaryDirectory temporary;
  std::filesystem::create_directories(temporary.path / "zeta");
  std::filesystem::create_directories(temporary.path / "alpha");
  touch(temporary.path / "zeta" / "second.p");
  touch(temporary.path / "alpha" / "two.p");
  touch(temporary.path / "alpha" / "one.p");
  touch(temporary.path / "alpha" / "ignored.txt");
  touch(temporary.path / "top-level.p");

  auto programs = benchmarking::discoverPrograms(temporary.path);

  ASSERT_EQ(programs.size(), 3);
  EXPECT_EQ(programs[0].group, "alpha");
  EXPECT_EQ(programs[0].name, "one");
  EXPECT_EQ(programs[1].group, "alpha");
  EXPECT_EQ(programs[1].name, "two");
  EXPECT_EQ(programs[2].group, "zeta");
  EXPECT_EQ(programs[2].name, "second");
}

TEST(BenchmarkDiscovery, RejectsMissingAndEmptyRoots) {
  TemporaryDirectory temporary;
  EXPECT_THROW(benchmarking::discoverPrograms(temporary.path / "missing"),
               std::runtime_error);
  EXPECT_THROW(benchmarking::discoverPrograms(temporary.path),
               std::runtime_error);
}
