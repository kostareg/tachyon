#pragma once

#include <cstdint>
#include <stdexcept>

namespace tachyon::parser
{
/**
 * @brief basic mathematical operators
 */
enum class Op
{
    Add,
    Sub,
    Mul,
    Div,
    Pow
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
    default:
        return "unknown";
    }
}

/**
 * @brief convert op to vm bytecode constant operations
 */
inline uint8_t opToUint8T(Op op)
{
    if (op == Op::Pow)
        throw std::runtime_error("power operator not yet supported");
    return static_cast<uint8_t>(op) + 0x50;
}
} // namespace tachyon::parser