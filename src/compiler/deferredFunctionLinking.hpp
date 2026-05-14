#pragma once

#include "expression.hpp"
#include <functional>

// Custom hasher and equality
namespace std {
template <> struct hash<std::reference_wrapper<FunctionExpression>> {
  std::size_t operator()(
      const std::reference_wrapper<FunctionExpression> &defer) const noexcept {
    // Reinterpret the memory address as a size_t
    return reinterpret_cast<size_t>(&defer.get());
  }
};

template <> struct equal_to<std::reference_wrapper<FunctionExpression>> {
  bool operator()(
      const std::reference_wrapper<FunctionExpression> &a,
      const std::reference_wrapper<FunctionExpression> &b) const noexcept {
    return &a.get() == &b.get();
  }
};
} // namespace std