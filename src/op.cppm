export module op;

/**
 * @brief Mathematical operators.
 */
export enum class Op { Add, Sub, Mul, Div, Pow };

/**
 * @brief For printing operators.
 * @sa Op
 */
export inline const char *op_to_str(Op op) {
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
