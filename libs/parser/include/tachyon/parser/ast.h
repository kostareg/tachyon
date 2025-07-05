#pragma once

#include "tachyon/common/error.h"
#include "tachyon/common/matrix.h"
#include "tachyon/common/op.h"

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
 * @brief literal values ie unit, numbers, strings, booleans, matrices
 */
using LiteralValue = std::variant<std::monostate, double, std::string, bool, Matrix>;

/**
 * @brief a literal expression, or constant value
 */
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
    List,
};

// fwd-decl
struct FunctionConcreteTypes;

/**
 * @brief the "Type" value, ie a reference to a concrete type or enum/struct
 */
using Type = std::variant<BasicConcreteTypes, FunctionConcreteTypes, std::string>;

/**
 * @brief optionally a type, for pre-inference abstract syntax tree
 */
using MaybeType = std::optional<Type>;

/**
 * @brief function type, eg Fn(Num) -> Num
 */
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
 * @brief <op> expr, eg !false
 */
struct UnaryOperatorExpr
{
    Op op;
    ExprRef right;
};

/**
 * @brief expr <op> expr, eg false || true
 */
struct BinaryOperatorExpr
{
    Op op;
    ExprRef left;
    ExprRef right;
};

/**
 * @brief let binding (variable)
 */
struct LetExpr
{
    std::string name;
    ExprRef value;
};

/**
 * @brief let binding reference (variable reference)
 */
struct LetRefExpr
{
    std::string name;
};

struct MatrixAssignmentExpr
{
    std::string name;
    ExprRef row;
    ExprRef col;
    ExprRef value;
};

struct MatrixRefExpr
{
    LetRefExpr ref;
    ExprRef row;
    ExprRef col;
};

// TODO: captures
/**
 * @brief function call
 */
struct FnCallExpr
{
    LetRefExpr ref;
    std::vector<Expr> args;
};

/**
 * @brief while loop
 */
struct WhileLoopExpr
{
    ExprRef condition;
    ExprRef body;
};

/**
 * @brief loop break
 */
struct BreakExpr
{
    ExprRef returns;
};

/**
 * @brief loop continue
 */
struct ContinueExpr
{
};

/**
 * @brief module import
 */
struct ImportExpr
{
    std::string path;
};

// TODO: retv
/**
 * @brief return value from function
 */
struct ReturnExpr
{
    ExprRef returns;
};

struct MatrixConstructExpr
{
    size_t height;
    std::vector<Expr> list;
};

/**
 * @brief linear sequence of expressions
 *
 * This is how we can show the idea of "statements" in the abstract syntax tree without moving too
 * far away from expressions. Any tree that needs a list of statements (e.g. function definition)
 * can keep a SequenceExpr that in turn keeps the list of expressions.
 */
struct SequenceExpr
{
    Exprs sequence;
};

/**
 * @brief type of expression
 */
using ExprKind =
    std::variant<LiteralExpr, FnExpr, UnaryOperatorExpr, BinaryOperatorExpr, LetExpr, LetRefExpr,
                 MatrixAssignmentExpr, MatrixRefExpr, FnCallExpr, WhileLoopExpr, BreakExpr,
                 ContinueExpr, ImportExpr, ReturnExpr, MatrixConstructExpr, SequenceExpr>;

/**
 * @brief top-level expression type
 */
struct Expr
{
    /// kind
    ExprKind kind;

    /// source span
    SourceSpan span;

    // TODO: remove?
    Expr(ExprKind kind) : kind(std::move(kind)), span(0, 0) {}
    Expr(ExprKind kind, SourceSpan span) : kind(std::move(kind)), span(span) {}
};

// TODO: type checking, inference. May also use type erasure just before
//  bytecode generation, that way BytecodeGenerator for FnExpr doesn't even know
//  the type of the arguments for example.
} // namespace tachyon::parser
