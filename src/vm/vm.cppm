module;

#include <cstdint>
#include <expected>
#include <variant>
#include <vector>

export module vm;

import error;

namespace vm {
export struct Proto;

// TODO: currently 32 bytes, can we get less?
//  if we use char* copy = malloc(strlen(original) + 1), can store char* instead
//  and get significant decrease (should be 16 bytes). and if we can store
//  7 byte numbers, even if they need alignment, we will probably be at 8 bytes
//  total size.
export using Value =
    std::variant<std::monostate, double, std::string, bool, const Proto *>;

struct Proto {
    // TODO: uint8_t* + malloc
    std::vector<uint8_t> bytecode;

    /**
     * @brief constant value lookup table
     */
    std::vector<Value> constants;

    // debug info
    std::string name;
    SourceSpan span;

    // TODO: when capturing vars from outer scope, just store a lookup table,
    //       don't store names
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

struct CallFrame {
    uint8_t returns;

    // TODO: maybe optimization point. store large values in their own regs?
    std::array<Value, 256> registers;
};

export class VM {
    std::vector<CallFrame> call_stack;

  public:
    VM() {
        auto initial_frame = CallFrame{};
        call_stack.push_back(std::move(initial_frame));
    }

    std::expected<void, Error> run(const Proto &proto);

    void diagnose() const;
};

/**
 * @brief bytecode values
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
 * | 0x00F0       | Function        |
 * +--------------+-----------------+
 */
export enum Bytecode : uint8_t {
    EXIT = 0x00, // exit
    NOOP = 0x01, // no-op
    RETV = 0x02, // return void
    RETC = 0x03, // return constant
    RETR = 0x04, // return register

    LOCR = 0x10, // load constant -> register
    LORR = 0x11, // load register -> register
    // LORS = 0x12, // load register <- stack

    // PUSC = 0x20, // push stack <- constant
    // PUSR = 0x21, // push stack <- register
    // POPS = 0x2E, // pop stack

    CRLC = 0x30, // (register  < constant) -> register
    CRGC = 0x31, // (register  > constant) -> register
    CRLR = 0x32, // (register  < register) -> register
    CRGR = 0x33, // (register  > register) -> register
    CREC = 0x34, // (register == constant) -> register
    CRER = 0x35, // (register == register) -> register

    JMPC = 0x40, // jump to constant address
    JMPR = 0x41, // jump to register address
    // JMPS = 0x42, // jump to stack address

    // TODO: if statements

    MARC = 0x50, // register + constant -> register
    MSRC = 0x51, // register - constant -> register
    MMRC = 0x52, // register * constant -> register
    MDRC = 0x53, // register / constant -> register
    MPRC = 0x54, // register ^ constant -> register
    MACR = 0x55, // constant + register -> register
    MSCR = 0x56, // constant - register -> register
    MMCR = 0x57, // constant * register -> register
    MDCR = 0x58, // constant / register -> register
    MPCR = 0x59, // constant ^ register -> register
    MARR = 0x5A, // register + register -> register
    MSRR = 0x5B, // register - register -> register
    MMRR = 0x5C, // register * register -> register
    MDRR = 0x5D, // register / register -> register
    MPRR = 0x5E, // register ^ register -> register

    // TODO: negate and inverse, potentially shortcuts for square and cube

    CALR = 0xF0, // call register
    PUHC = 0xF1, // push constant to next function call
    PUHR = 0xF2, // push register to next function call
};
} // namespace vm
