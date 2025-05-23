module;

#include <cstdint>
#include <expected>
#include <stack>
#include <vector>

export module vm;

import error;

namespace vm {
/*
 * @brief Function prototype.
 */
export struct Proto {
    /// Function name.
    std::string name;

    /// Compiled virtual bytecode.
    uint16_t *bc;

    /// Whether the function returns anything.
    bool returns =
        true; // TODO: remove this and make it declared in the AST or IR.

    /// List of children.
    std::vector<std::unique_ptr<Proto>> children;

    Proto(std::string name, uint16_t *bc) : name(name), bc(bc) {};
    Proto(std::string name, uint16_t *bc,
          std::vector<std::unique_ptr<Proto>> children)
        : name(name), bc(bc), children(std::move(children)) {};
    Proto(std::string name, uint16_t *bc,
          std::vector<std::unique_ptr<Proto>> children, bool returns)
        : name(name), bc(bc), returns(returns),
          children(std::move(children)) {};
};

class RegisterAllocator {
    /**
     * @brief 16-bit registers.
     */
    uint16_t *registers;

    /**
     * @brief length of registers.
     */
    size_t reglast;

    /**
     * @brief register capacity.
     */
    size_t cap;

  public:
    RegisterAllocator(size_t initial_cap = 1024) : cap(initial_cap) {
        registers = static_cast<uint16_t *>(malloc(cap * sizeof(uint16_t)));
        reglast = 0;
    }

    uint16_t *new_fn() {
        space_fn();

        uint16_t *r = registers + reglast;
        reglast += 256;
        return r;
    }

    /**
     * Pushes one value to the start of the next set of registers.
     */
    void early_push(size_t off, uint16_t val) {
        space_fn();

        registers[reglast + off] = val;
    }

    /*
     * @brief Make sure there's enough space for an upcoming function.
     */
    void space_fn() {
        if (reglast >= cap) {
            cap *= 2;
            registers = static_cast<uint16_t *>(
                realloc(registers, cap * sizeof(uint16_t)));
        }
    }

    void free_fn() {
        reglast -= 256;

        if (cap / 2 >= reglast) {
            cap /= 2;
            registers = static_cast<uint16_t *>(
                realloc(registers, cap * sizeof(uint16_t)));
        }
    }

    uint16_t &get_last_fn(size_t i) { return registers[reglast - 256 * 2 + i]; }

    /**
     * @brief Always point to the last 256 registers.
     */
    uint16_t &operator[](size_t i) { return registers[reglast - 256 + i]; }

    ~RegisterAllocator() { free(registers); }
};

/**
 * @brief Just-in-time virtual machine.
 */
export class VM {
    /**
     * @brief Holds a pointer to all registers and reallocs/frees when needed.
     * @sa RegisterAllocator
     */
    RegisterAllocator registers;

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
    std::expected<void, Error> run_fn(Proto *proto);
};

enum Bytecode {
    EXIT = 0x0000,
    NOOP = 0x0001,
    RETV = 0x0002,
    RETC = 0x0003,
    RETR = 0x0004,

    LORC = 0x0010,
    LORR = 0x0011,
    LORS = 0x0012,

    PUSC = 0x0020,
    PUSR = 0x0021,
    POPS = 0x002E,

    CRLC = 0x0030,
    CRGC = 0x0031,
    CRLR = 0x0032,
    CRGR = 0x0033,
    CREC = 0x0034,
    CRER = 0x0035,

    JMPC = 0x0040,
    JMPR = 0x0041,
    JMPS = 0x0042,

    // TODO: if statements

    MARC = 0x0050,
    MSRC = 0x0051,
    MMRC = 0x0052,
    MDRC = 0x0053,
    MPRC = 0x0054,
    MACR = 0x0055,
    MSCR = 0x0056,
    MMCR = 0x0057,
    MDCR = 0x0058,
    MPCR = 0x0059,
    MARR = 0x005A,
    MSRR = 0x005B,
    MMRR = 0x005C,
    MDRR = 0x005D,
    MPRR = 0x005E,

    // TODO: negate and inverse, potentially shortcuts for square and cube

    CALR = 0x0100,
    PUHC = 0x0110,
    PUHR = 0x0111,
};
} // namespace vm
