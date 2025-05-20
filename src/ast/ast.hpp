#pragma once

#include <expected>
#include <memory>
#include <vector>

#include "error.hpp"
#include "vm/vm.hpp"

/**
 * @namespace ast
 * @brief Abstract syntax tree nodes.
 */
namespace ast {
// fwd-decl
struct Expr;
using ExprRef = std::unique_ptr<Expr>;
using ExprRefs = std::vector<std::unique_ptr<Expr>>;

// kinds of exprs

struct LiteralExpr {
    int myint;
};

using ExprKind = std::variant<LiteralExpr>;

/**
 * @brief Top-level expression type.
 */
struct Expr {
    SourceSpan span;
    ExprKind kind;
};

std::expected<vm::Proto, Error> generate_proto(ExprRefs);
} // namespace ast
