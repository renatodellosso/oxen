#pragma once

#include "scope.hpp"
#include "value.hpp"
#include <atomic>
#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <vector>

class Subprogram;

enum class InstructionType {
  Block,
  GetLiteral,
  ReferenceIdentifier,
  GetIdentifier,
  Declare,
  Set,
  Add,
  Subtract,
  Multiply,
  Divide,
  Negate,
  CompareEquals,
  CompareLessThan,
  CompareLessThanEquals,
  CompareGreaterThan,
  CompareGreaterThanEquals,
  If,
  While,
  GoTo,
  Print,
  Function,
  Call,
  CompareNotEquals,
  Else,
  BranchMerge,
  Return
};

std::string instructionTypeToString(InstructionType type);

struct Instruction;
struct ReturnInvocation;
struct CallCompletion;

struct InstrDependent {
  Instruction *instr;
  std::optional<int> argIndex;

  bool disabled;
  bool completionBarrierRemapped;
  std::shared_ptr<ReturnInvocation> returnInvocation;
  std::shared_ptr<CallCompletion> callCompletion;

  InstrDependent(Instruction *instr, std::optional<int> argIndex);
  InstrDependent(Instruction *instr, int argIndex);
  InstrDependent(Instruction *instr);
};

// A function body's instructions are cloned for every call. Keep both the
// winning-return latch and the destinations it releases on that cloned call,
// rather than on the reusable dependency edges of the Call instruction.
struct ReturnInvocation {
  std::uint64_t id;
  std::atomic_bool claimed;
  std::vector<InstrDependent> dependents;

  ReturnInvocation(std::uint64_t id,
                   std::vector<InstrDependent> dependents);
};

// Collects all resource and terminal-effect signals for one remapped call
// dependency, then releases the caller exactly once.
struct CallCompletion {
  std::uint64_t invocationId;
  std::atomic_int remaining;
  InstrDependent dependent;
  std::shared_ptr<Value> result;

  CallCompletion(std::uint64_t invocationId, int expectedSignals,
                 InstrDependent dependent);
};

struct Instruction {
  int id;

  InstructionType type;

  // Args inherent to the instruction
  std::vector<Value> bytecodeArgs;
  // Args from previous instructions
  std::vector<std::shared_ptr<Value>> depArgs;

  bool skipped = false;
  bool queued = false;
  int depCount, depsFulfilled;
  std::vector<InstrDependent> dependents;

  std::shared_ptr<Scope<Value>> scope;

  std::shared_ptr<Subprogram> program;

  // Has this instruction been executed? NOT reset if re-executed
  bool executed;

  Instruction(int id, std::shared_ptr<Scope<Value>> scope = nullptr);

  std::string toString();
};
