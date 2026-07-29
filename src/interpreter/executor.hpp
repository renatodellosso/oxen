#pragma once

#include "../cliUtils.hpp"
#include "../concurrentQueue.hpp"
#include "../instruction.hpp"
#include "subprogram.hpp"
#include <atomic>
#include <cstdint>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#define THREADS_PER_QUEUE 64

struct ExecutionStats {
  std::uint64_t executedInstructions;

  ExecutionStats();
};

class Executor {
  const CliArgs &cliArgs;
  Subprogram &program;
  ExecutionStats *stats;
  std::vector<ConcurrentQueue<std::reference_wrapper<Instruction>>> queues;
  std::vector<std::thread> workers;
  std::vector<std::uint64_t> executedInstructionsByWorker;
  // Instructions currently being executed or waiting on the queue
  std::atomic_int pendingInstructions;
  // Used to track monotonically increasing IDs for each function call
  std::atomic_uint64_t nextCallInvocationId;

  // We have to put the mutexes in here since we can't move them
  std::vector<std::mutex> depArgsMutexes, depsFulfilledMutexes;

  // Loop-back instructions reset dependency state across many instructions.
  // Keep that reset atomic with respect to dependency publication.
  // Recursive because skipping a block recursively skips its instructions and
  // may publish dependencies while the outer skip operation is still active.
  std::recursive_mutex dependencyStateMutex;
  std::mutex coutMutex;

  // Set to true to end workers
  std::atomic_bool halt;
  std::atomic_bool failed;
  std::string haltCause;

  int getQueueId(int worker, int offset) const;

  // Increment depsFulfilled and, if relevant, sets depArgs[i]
  void updateDependency(InstrDependent dep, std::shared_ptr<Value> result);
  void enqueueIfReady(Instruction &instr);
  // Use recurse = true at the root level for skipping blocks
  void skipInstruction(Instruction &instr, bool markSkippedAs = true);
  void execSingleInstruction(Instruction &instr,
                             std::uint64_t &executedInstructions);

  // Multithreaded worker that actually executes instructions
  void execWorker(int id);
  // Waits until all instructions have been executed and then halts workers
  void supervisor();

  // Reads instructions and pushes everything that's ready onto the queue
  void initQueues();
  void initScopes();

public:
  Executor(const CliArgs &cliArgs, Subprogram &program,
           ExecutionStats *stats = nullptr);
  void startExecution();
};
