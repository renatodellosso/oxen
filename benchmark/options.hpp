#pragma once

#include "types.hpp"
#include <string_view>
#include <vector>

Options parseOptions(const std::vector<std::string_view> &arguments);
