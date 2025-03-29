#pragma once

/**
 * @brief Mathematical operators.
 */
enum class Op { Add, Sub, Mul, Div, Pow };

/**
 * @brief For printing operators.
 * @sa Op
 */
inline const char *op_to_str(Op op) {
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

/**
 * @brief Operator precedence, higher goes first.
 */
inline int precedence(Op op) {
    switch (op) {
    case Op::Add:
    case Op::Sub:
        return 1;
    case Op::Mul:
    case Op::Div:
        return 2;
    case Op::Pow:
        return 3;
    }
}
