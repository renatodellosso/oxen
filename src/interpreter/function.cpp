#include "function.hpp"
#include "subprogram.hpp"
#include <vector>

Function::Function(Instruction &instr, Subprogram &instructions) {
  returnType = std::get<std::string>(instr.bytecodeArgs[0].val);
  name = std::get<std::string>(instr.bytecodeArgs[1].val);
  generatedLoopBody = instr.bytecodeArgs.size() > 2 &&
                      instr.bytecodeArgs[2].type == ValueType::Bool &&
                      std::get<bool>(instr.bytecodeArgs[2].val);

  auto block = instructions[instr.id + 1];
  auto size =
      std::get<int>(block.bytecodeArgs[0].val) + 1; // +1 for the block itself
  body = Subprogram(instructions, block.id, size);
}

std::string Function::getName() const { return name; }
std::string Function::getReturnType() const { return returnType; }
bool Function::isGeneratedLoopBody() const { return generatedLoopBody; }

Subprogram &Function::getBody() { return body; }
