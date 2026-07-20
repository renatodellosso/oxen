#include "discovery.hpp"
#include <algorithm>
#include <format>
#include <stdexcept>

namespace benchmarking {

std::vector<Program> discoverPrograms(const std::filesystem::path &root) {
  if (!std::filesystem::exists(root))
    throw std::runtime_error(
        std::format("Benchmark folder '{}' does not exist", root.string()));
  if (!std::filesystem::is_directory(root))
    throw std::runtime_error(
        std::format("Benchmark root '{}' is not a directory", root.string()));

  std::vector<Program> programs;
  for (const auto &groupEntry : std::filesystem::directory_iterator(root)) {
    if (!groupEntry.is_directory())
      continue;

    const std::string group = groupEntry.path().filename().string();
    for (const auto &programEntry :
         std::filesystem::directory_iterator(groupEntry.path())) {
      if (!programEntry.is_regular_file() ||
          programEntry.path().extension() != ".p")
        continue;

      programs.push_back({.group = group,
                          .name = programEntry.path().stem().string(),
                          .path = programEntry.path()});
    }
  }

  std::ranges::sort(programs, [](const Program &left, const Program &right) {
    if (left.group != right.group)
      return left.group < right.group;
    return left.name < right.name;
  });

  if (programs.empty())
    throw std::runtime_error(std::format(
        "No .p benchmark programs found under '{}'", root.string()));

  return programs;
}

} // namespace benchmarking
