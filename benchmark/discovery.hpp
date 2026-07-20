#pragma once

#include "types.hpp"
#include <filesystem>
#include <vector>

namespace benchmarking {

std::vector<Program> discoverPrograms(const std::filesystem::path &root);

} // namespace benchmarking
