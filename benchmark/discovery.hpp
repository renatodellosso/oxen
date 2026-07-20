#pragma once

#include "types.hpp"
#include <filesystem>
#include <vector>

std::vector<Program> discoverPrograms(const std::filesystem::path &root);
