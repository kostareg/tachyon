#pragma once

#include <cstdint>
#include <stdexcept>

/**
 * @brief Mathematical operators.
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
 * @brief For printing operators.
 * @sa Op
 */
inline const char *op_to_str(Op op)
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
 * @brief convert op to vm bytecode constant operations.
 */
inline const uint8_t op_to_uint8_t(Op op)
{
    if (op == Op::Pow)
        throw std::runtime_error("power operator not yet supported");
    return static_cast<uint8_t>(op) + 0x50;
}