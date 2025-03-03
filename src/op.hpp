#pragma once

#include <string>

/**
 * @brief Mathematical operators.
 */
enum class Op { Add, Sub, Mul, Div, Pow };

/**
 * @brief For printing operators.
 * @sa Op
 */
inline std::string op_to_string(Op op) {
    switch (op) {
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
