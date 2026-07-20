#include "error.hpp"

Error::Error(ExitCode exitCode, const std::string &message)
    : std::runtime_error(message), exitCode(exitCode) {}

ExitCode Error::getCode() const noexcept { return exitCode; }
