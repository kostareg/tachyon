#pragma once

#include <cstdint>
#include <mimalloc.h>
#include <stack>

#include "proto.hpp"
#include "register_allocator.hpp"

// TODO: mmap or malloc instead of arrays?

/**
 * @brief Just-in-time virtual machine.
 */
class VM {
    /**
     * @brief Holds a pointer to all registers and reallocs/frees when needed.
     * @sa RegisterAllocator
     */
    RegisterAllocator regalloc;

    /**
     * @brief The stack.
     * @see VM::run() for details on pushing and popping from stack.
     */
    std::stack<uint16_t> stack;

    std::vector<std::unique_ptr<Proto>> fns;

  public:
    /**
     * @brief Run the virtual machine interpreter.
     *
     * +--------------+-----------------+
     * | Start        | Operation class |
     * +--------------+-----------------+
     * | 0x0000       | Machine         |
     * | 0x0010       | Register        |
     * | 0x0020       | Stack           |
     * | 0x0030       | Comparison      |
     * | 0x0040       | Positional      |
     * | 0x0050       | Arithmetic      |
     * | 0x0100       | Function        |
     * +--------------+-----------------+
     */
    void run_fn(Proto *proto);
};
