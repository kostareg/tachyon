#pragma once

#include <cstdint>

// TODO: check naming convention for enum class values
namespace tachyon::parser
{
/**
 * @brief basic operators
 */
enum class Op
{
    /* boolean operators */
    Eq,
    Neq,
    Lst,
    Grt,
    Lset,
    Gret,

    /* number operators */
    Add,
    Sub,
    Mul,
    Div,
    Pow,
};

/**
 * @brief show operator
 * @sa Op
 */
inline const char *opToStr(Op op)
{
    switch (op)
    {
    case Op::Add:
        return "+";
    case Op::Sub:
        return "-";
    case Op::Mul:
        return "*";
    case Op::Div:
        return "/";
    case Op::Pow:
        return "^";
    case Op::Eq:
        return "==";
    case Op::Neq:
        return "!=";
    case Op::Lst:
        return "<";
    case Op::Grt:
        return ">";
    case Op::Lset:
        return "<=";
    case Op::Gret:
        return ">=";
    default:
        return "unknown";
    }
}

/**
 * @brief convert op to vm bytecode constant operations
 */
inline uint8_t opToUint8T(Op op)
{
    if (auto num = static_cast<uint8_t>(op); num < 6)
        return num + 0x70;
    else
        return num - 6 + 0x50;
}

// TODO: this is not a good solution.
/**
 * @brief get nth next group of bytecode
 */
inline uint8_t getBytecodeNthGroup(uint8_t addr, int n)
{
    if (addr < 0x70)
        return addr + 5 * n;
    else
        return addr + 6 * n;
}
} // namespace tachyon::parser