#pragma once

#include "types.hpp"
#include <vector>

namespace benchmarking {

ProgramSummary summarize(const std::vector<TrialResult> &trials);

} // namespace benchmarking
