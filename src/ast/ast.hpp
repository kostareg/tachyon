#pragma once

#include <expected>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "error.hpp"
#include "op.hpp"
#include "vm/vm.hpp"

/**
 * @namespace ast
 * @brief abstract syntax tree nodes
 */
namespace ast {
// fwd-decl
struct Expr;
using ExprRef = std::unique_ptr<Expr>;
using Exprs = std::vector<Expr>;

/**
 * @brief literal values, ie strings, numbers, booleans, unit
 */
using LiteralValue = std::variant<double, std::string, bool, std::monostate>;

struct LiteralExpr {
    LiteralValue value;
};

/**
 * @brief four basic concrete types.
 */
enum class BasicConcreteTypes {
    Number,
    String,
    Boolean,
    Unit,
};

// fwd-decl
struct FunctionConcreteTypes;

/**
 * @brief the "Type" value, ie a reference to a concrete type or enum/struct
 */
using Type =
    std::variant<BasicConcreteTypes, FunctionConcreteTypes, std::string>;

using MaybeType = std::optional<Type>;

struct FunctionConcreteTypes {
    std::unordered_map<std::string, Type> arguments;
};

/**
 * @brief a function declaration expression, eg `fn(x) { x + 1 }`
 *
 * All functions are unnamed by default in Tachyon, so this structure only
 * needs three values: the arguments, the return value, and the body. The
 * arguments are an unordered map of the name and type. Since this is the weak
 * side of the AST, types can be left blank to be inferred (std::nullopt).
 */
struct FnExpr {
    std::unordered_map<std::string, MaybeType> arguments;
    MaybeType returns;
    ExprRef body;
};

/**
 * @brief <expr> +, -, *, /, ^ <expr>
 */
struct BinaryOperatorExpr {
    Op op;
    ExprRef left;
    ExprRef right;
};

struct LetExpr {
    std::string name;
    ExprRef value;
};

struct LetRefExpr {
    std::string name;
};

struct ImportExpr {
    std::string path;
};

struct ReturnExpr {
    ExprRef returns;
};

struct SequenceExpr {
    Exprs sequence;
};

using ExprKind = std::variant<LiteralExpr, FnExpr, BinaryOperatorExpr, LetExpr,
                              LetRefExpr, ImportExpr, ReturnExpr, SequenceExpr>;

/**
 * @brief Top-level expression type.
 */
struct Expr {
    SourceSpan span;
    ExprKind kind;
};

std::expected<vm::Proto, Error> generate_proto(Exprs);
} // namespace ast
