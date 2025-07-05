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
    And,
    Or,
    Not,

    /* comparison operators */
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
    case Op::And:
        return "&&";
    case Op::Or:
        return "||";
    case Op::Not:
        return "!";
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
    default:
        return "unknown";
    }
}

/**
 * @brief convert op to vm bytecode constant operations
 *
 * Doesn't support Op::Not, since it's the only unary operator.
 */
inline uint16_t opToUint16T(Op op)
{
    if (auto num = static_cast<uint16_t>(op); num < 3)
        return num + 0x32;
    else if (num < 9)
        return num - 3 + 0x70;
    else
        return num - 9 + 0x50;
}

// TODO: this is not a good solution.
/**
 * @brief get nth next group of bytecode
 */
inline uint16_t getBytecodeNthGroup(uint16_t addr, int n)
{
    if (addr < 0x50) // boolean operators
        return addr + 2 * n;
    if (addr < 0x70) // comparison operators
        return addr + 5 * n;
    else // number operators
        return addr + 6 * n;
}
} // namespace tachyon::parser
