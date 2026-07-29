#include "instruction.hpp"
#include "color.hpp"
#include <format>
#include <memory>
#include <string>
#include <utility>

std::string instructionTypeToString(InstructionType type) {
  switch (type) {
  case InstructionType::Block:
    return "Block";
  case InstructionType::GetLiteral:
    return "GetLiteral";
  case InstructionType::ReferenceIdentifier:
    return "ReferenceIdentifier";
  case InstructionType::GetIdentifier:
    return "GetIdentifier";
  case InstructionType::Declare:
    return "Declare";
  case InstructionType::Set:
    return "Set";
  case InstructionType::Add:
    return "Add";
  case InstructionType::Subtract:
    return "Subtract";
  case InstructionType::Multiply:
    return "Multiply";
  case InstructionType::Divide:
    return "Divide";
  case InstructionType::Negate:
    return "Negate";
  case InstructionType::CompareEquals:
    return "CompareEquals";
  case InstructionType::CompareNotEquals:
    return "CompareNotEquals";
  case InstructionType::CompareLessThan:
    return "CompareLessThan";
  case InstructionType::CompareLessThanEquals:
    return "CompareLessThanEquals";
  case InstructionType::CompareGreaterThan:
    return "CompareGreaterThan";
  case InstructionType::CompareGreaterThanEquals:
    return "CompareGreaterThanEquals";
  case InstructionType::If:
    return "If";
  case InstructionType::While:
    return "While";
  case InstructionType::GoTo:
    return "GoTo";
  case InstructionType::Print:
    return "Print";
  case InstructionType::Function:
    return "Function";
  case InstructionType::Call:
    return "Call";
  case InstructionType::Else:
    return "Else";
  case InstructionType::BranchMerge:
    return "BranchMerge";
  case InstructionType::Return:
    return "Return";
  default:
    return std::format("UnknownInstructionType:{}", (int)type);
  }
}

InstrDependent::InstrDependent(Instruction *instr, std::optional<int> argIndex)
    : instr(instr), argIndex(argIndex), disabled(false),
      returnInvocation(nullptr), callCompletion(nullptr) {}

InstrDependent::InstrDependent(Instruction *instr, int argIndex)
    : InstrDependent(instr, std::make_optional(argIndex)) {}

InstrDependent::InstrDependent(Instruction *instr)
    : InstrDependent(instr, std::nullopt) {}

ReturnInvocation::ReturnInvocation(std::uint64_t id,
                                   std::vector<InstrDependent> dependents)
    : id(id), claimed(false), dependents(std::move(dependents)) {}

CallCompletion::CallCompletion(std::uint64_t invocationId, int expectedSignals,
                               InstrDependent dependent)
    : invocationId(invocationId), remaining(expectedSignals),
      dependent(std::move(dependent)), result(nullptr) {}

Instruction::Instruction(int id, std::shared_ptr<Scope<Value>> scope)
    : id(id), type((InstructionType)0), bytecodeArgs(std::vector<Value>()),
      depArgs(std::vector<std::shared_ptr<Value>>()), depCount(0),
      depsFulfilled(0), dependents(std::vector<InstrDependent>()),
      scope(scope) {}

std::string Instruction::toString() {
  auto depCountStr =
      std::format("{}/{}", depsFulfilled.load(), depCount.load());
  if (depsFulfilled == depCount)
    depCountStr = colorize(depCountStr, Color::Green);
  else
    depCountStr = colorize(depCountStr, Color::Yellow);

  std::string str = std::format(
      "({}){}(dependencies: {}, scope depth: {}, bytecode args: [",
      colorize(std::to_string(id), Color::Cyan), instructionTypeToString(type),
      depCountStr, scope ? scope->getDepth() : -1);

  for (auto arg : bytecodeArgs)
    str += valToStr(arg, true) + ", ";
  if (bytecodeArgs.size() > 0)
    str = str.substr(0, str.length() - 2);
  str += "], dep args: [";

  for (auto arg : depArgs) {
    if (arg)
      str += valToStr(*arg, true) + ", ";
    else
      str += "<error - nullptr value>, ";
  }
  if (depArgs.size() > 0)
    str = str.substr(0, str.length() - 2);
  str += "], dependents: [";

  for (auto dep : dependents) {
    auto depStr = std::to_string(dep.instr->id);
    if (dep.argIndex.has_value())
      depStr += "." + std::to_string(dep.argIndex.value());

    if (dep.disabled)
      depStr = colorize(depStr, Color::Red);

    str += depStr + ", ";
  }
  if (dependents.size() > 0)
    str = str.substr(0, str.length() - 2);
  str += "]";

  return str + ")";
}

#define INSTR_CONSTRUCTOR : id(other.id), type(other.type), bytecodeArgs(other.bytecodeArgs), \
    depArgs(other.depArgs), depCount(other.depCount.load()), \
    depsFulfilled(other.depsFulfilled.load()), dependents(other.dependents), \
    scope(other.scope), skipped(other.skipped), queued(other.queued), \
     executed(other.executed), program(other.program) \
    {}

#define INSTR_ASSIGN_CONSTRUCTOR                                               \
  {                                                                            \
    if (&other == this)                                                        \
      return *this;                                                            \
                                                                               \
    id = other.id;                                                             \
    type = other.type;                                                         \
    bytecodeArgs = other.bytecodeArgs;                                         \
    depArgs = other.depArgs;                                                   \
    depCount = other.depCount.load();                                          \
    depsFulfilled = other.depsFulfilled.load();                                \
    dependents = other.dependents;                                             \
    scope = other.scope;                                                       \
    skipped = other.skipped;                                                   \
    queued = other.queued;                                                     \
    executed = other.executed;                                                 \
    program = other.program;                                                   \
                                                                               \
    return *this;                                                              \
  }

Instruction::Instruction(const Instruction &other) INSTR_CONSTRUCTOR

    Instruction &Instruction::operator=(const Instruction &other)
        INSTR_ASSIGN_CONSTRUCTOR

    // Not sure why it indents this
    Instruction::Instruction(Instruction &&other) noexcept INSTR_CONSTRUCTOR

    Instruction &Instruction::operator=(Instruction &&other) noexcept
    INSTR_ASSIGN_CONSTRUCTOR