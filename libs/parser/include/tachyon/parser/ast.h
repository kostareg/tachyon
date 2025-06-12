#pragma once

#include "tachyon/common/error.h"
#include "tachyon/parser/op.h"

#include <memory>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

namespace tachyon::parser
{
// fwd-decl
struct Expr;
using ExprRef = std::unique_ptr<Expr>;
using Exprs = std::vector<Expr>;

/**
 * @brief literal values, ie unit, numbers, strings, booleans
 */
using LiteralValue = std::variant<std::monostate, double, std::string, bool>;

struct LiteralExpr
{
    LiteralValue value;
};

/**
 * @brief four basic concrete types.
 */
enum class BasicConcreteTypes
{
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
using Type = std::variant<BasicConcreteTypes, FunctionConcreteTypes, std::string>;

using MaybeType = std::optional<Type>;

struct FunctionConcreteTypes
{
    std::unordered_map<std::string, Type> arguments;
};

/**
 * @brief a function declaration expression, eg `fn(x) { x + 1 }`
 *
 * All functions are unnamed by default in Tachyon, so this structure only needs three values: the
 * arguments, the return value, and the body. The arguments are an unordered map of the name and
 * type. Since this is the weak side of the AST, types can be left blank to be inferred
 * (std::nullopt).
 */
struct FnExpr
{
    std::vector<std::pair<std::string, MaybeType>> arguments;
    MaybeType returns;
    ExprRef body;
};

/**
 * @brief <expr> +, -, *, /, ^ <expr>
 */
struct BinaryOperatorExpr
{
    Op op;
    ExprRef left;
    ExprRef right;
};

struct LetExpr
{
    std::string name;
    ExprRef value;
};

struct LetRefExpr
{
    std::string name;
};

// TODO: captures
struct FnCallExpr
{
    LetRefExpr ref;
    std::vector<Expr> args;
};

struct ImportExpr
{
    std::string path;
};

// TODO: retv
struct ReturnExpr
{
    ExprRef returns;
};

struct SequenceExpr
{
    Exprs sequence;
};

using ExprKind = std::variant<LiteralExpr, FnExpr, BinaryOperatorExpr, LetExpr, LetRefExpr,
                              FnCallExpr, ImportExpr, ReturnExpr, SequenceExpr>;

/**
 * @brief Top-level expression type.
 */
struct Expr
{
    ExprKind kind;
    SourceSpan span;

    // TODO: remove
    Expr(ExprKind kind) : kind(std::move(kind)), span(0, 0) {}
    Expr(ExprKind kind, SourceSpan span) : kind(std::move(kind)), span(span) {}
};

// TODO: type checking, inference. May also use type erasure just before
//  bytecode generation, that way BytecodeGenerator for FnExpr doesn't even know
//  the type of the arguments for example.
} // namespace tachyon::parser
