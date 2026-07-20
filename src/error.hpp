#pragma once

#include "exitCode.hpp"
#include <stdexcept>
#include <string>

// Associates a user-facing failure message with the process exit code that
// callers should return at an application boundary.
class Error : public std::runtime_error {
  ExitCode exitCode;

public:
  Error(ExitCode exitCode, const std::string &message);

  ExitCode getCode() const noexcept;
};
