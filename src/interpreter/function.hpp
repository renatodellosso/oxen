#pragma once

#include "../instruction.hpp"
#include "subprogram.hpp"
#include <string>

struct FunctionParam {
  std::string name, type;
};

class Function {
  std::string name;
  std::string returnType;

  Subprogram body;

public:
  Function(Instruction &instr, Subprogram &instructions);

  std::string getName() const;
  std::string getReturnType() const;

  Subprogram &getBody();
};