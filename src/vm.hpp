#pragma once

#include <array>
#include <cstdint>
#include <stack>

// TODO: mmap or malloc instead of arrays?

/**
 * @brief Just-in-time virtual machine.
 */
class VM {
  public:
    /**
     * @brief 32 16-bit global registers.
     *
     * All initialized to zero.
     */
    std::array<uint16_t, 32> registers = {};

    /**
     * @brief Program memory.
     *
     * The program memory is organized like so (subject to change):
     *
     * +----------+----------------+
     * | Position + Description    |
     * +----------+----------------+
     * | 0-7      | Header         |
     * | 8-65535  | Program        |
     * +----------+----------------+
     *
     * The program is loaded here using `VM::load`, then the program counter
     * goes through each instruction from the program section.
     */
    uint16_t memory[0xFFFF];

    /**
     * @brief The stack.
     * @see VM::run() for details on pushing and popping from stack.
     */
    std::stack<uint16_t> stack;

    /**
     * @brief Program counter.
     */
    uint16_t pc = 8;

    /**
     * @brief Load a program into memory.
     */
    void load(uint16_t program[0xFFFF]);

    /**
     * @brief Run the virtual machine interpreter.
     */
    void run();
};
