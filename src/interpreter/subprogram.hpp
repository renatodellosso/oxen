#pragma once

#include "../instruction.hpp"
#include <memory>
#include <vector>

// Be sure to call setSubprogramPointers!
class Subprogram {
  std::shared_ptr<std::vector<Instruction>> instrs;

public:
  Subprogram();
  Subprogram(Subprogram base, int startIndex, int size);
  Subprogram(std::shared_ptr<std::vector<Instruction>> instrs);

  void setSubprogramPointers(std::shared_ptr<Subprogram> self);

  Instruction &operator[](size_t i);

  // For use in range-based for loops (for each loops)
  std::vector<Instruction>::iterator begin();
  std::vector<Instruction>::iterator end();
  Instruction &at(int index);

  std::shared_ptr<Subprogram> clone() const;

  int size() const;
};