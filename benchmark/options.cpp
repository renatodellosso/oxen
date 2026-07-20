#include "options.hpp"
#include <charconv>
#include <format>
#include <stdexcept>
#include <string>

namespace {

[[noreturn]] void usageError(const std::string &message) {
  throw std::runtime_error(std::format(
      "{}\nUsage: benchmark [--trials N] [--threads LIST]\n"
      "  --trials N      Positive number of trials per program/thread count\n"
      "  --threads LIST  Comma-separated positive thread counts, e.g. 1,2,4,8",
      message));
}

int parsePositiveInt(std::string_view value, std::string_view optionName) {
  if (value.empty())
    usageError(std::format("{} must not be empty", optionName));

  int parsed = 0;
  const char *end = value.data() + value.size();
  auto result = std::from_chars(value.data(), end, parsed);
  if (result.ec != std::errc() || result.ptr != end || parsed <= 0)
    usageError(std::format("{} must be a positive integer", optionName));

  return parsed;
}

std::vector<int> parseThreadList(std::string_view value) {
  if (value.empty())
    usageError("--threads must not be empty");

  std::vector<int> threads;
  std::size_t start = 0;
  while (start <= value.size()) {
    std::size_t comma = value.find(',', start);
    std::size_t end = comma == std::string_view::npos ? value.size() : comma;
    std::string_view item = value.substr(start, end - start);
    if (item.empty())
      usageError("--threads must contain only positive integers");
    threads.push_back(parsePositiveInt(item, "--threads"));

    if (comma == std::string_view::npos)
      break;
    start = comma + 1;
  }

  return threads;
}

} // namespace

Options parseOptions(const std::vector<std::string_view> &arguments) {
  Options options;

  for (std::size_t i = 0; i < arguments.size(); ++i) {
    std::string_view argument = arguments[i];
    if (argument == "--trials") {
      if (++i == arguments.size())
        usageError("--trials requires a value");
      options.trials = parsePositiveInt(arguments[i], "--trials");
    } else if (argument == "--threads") {
      if (++i == arguments.size())
        usageError("--threads requires a value");
      options.threads = parseThreadList(arguments[i]);
    } else {
      usageError(std::format("Unknown option '{}'", argument));
    }
  }

  return options;
}
