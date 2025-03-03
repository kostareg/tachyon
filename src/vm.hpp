#pragma once

#include <cstdint>
#include <stack>
#include <string>
#include <unordered_map>

class VM {
  public:
    // TODO: mmap or malloc?
    uint16_t registers[32] = {};
    /// The program is loaded here.
    uint16_t memory[0xFFFF];
    std::stack<uint16_t> stack;
    /// Program counter.
    uint16_t pc = 8;

    void load(uint16_t program[0xFFFF]);
    void run();
};
