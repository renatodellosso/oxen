#pragma once

#include <ostream>

class ScopedStreamRedirect {
  std::ostream &stream;
  std::streambuf *original;

public:
  ScopedStreamRedirect(std::ostream &stream, std::streambuf *replacement)
      : stream(stream), original(stream.rdbuf(replacement)) {}

  ~ScopedStreamRedirect() { stream.rdbuf(original); }

  ScopedStreamRedirect(const ScopedStreamRedirect &) = delete;
  ScopedStreamRedirect &operator=(const ScopedStreamRedirect &) = delete;
};
