#pragma once

#include "types.hpp"
#include <string_view>
#include <vector>

namespace benchmarking {

Options parseOptions(const std::vector<std::string_view> &arguments);

} // namespace benchmarking
