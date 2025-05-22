#pragma once

#include <expected>
#include <memory>
#include <print>
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
 * @brief literal values, ie unit, numbers, strings, booleans
 */
using LiteralValue = std::variant<std::monostate, double, std::string, bool>;

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
    ExprKind kind;
    SourceSpan span;
};

std::expected<vm::Proto, Error> generate_proto(Expr);

// visitors
struct PrintLiteral {
    void operator()(const std::string &val) const { std::print("{}", val); }
    void operator()(double val) const { std::print("{}", val); }
    void operator()(bool val) const {
        std::print("{}", val ? "true" : "false");
    }
    void operator()(std::monostate _) const { std::print("()"); }
};

struct PrintType {
    void operator()(const BasicConcreteTypes &btyp) const {
        if (btyp == BasicConcreteTypes::Number)
            std::print("Num");
        else if (btyp == BasicConcreteTypes::String)
            std::print("Str");
        else if (btyp == BasicConcreteTypes::Boolean)
            std::print("Bool");
        else if (btyp == BasicConcreteTypes::Unit)
            std::print("()");
    }

    void operator()(const FunctionConcreteTypes &ftyp) const {
        for (auto pair : ftyp.arguments) {
            std::visit(*this, pair.second);
        }
    }

    void operator()(const std::string &otyp) const { std::print("{}", otyp); }
};

struct Printer {
    void operator()(const LiteralExpr &literal) const {
        std::visit(PrintLiteral{}, literal.value);
    }

    void operator()(const FnExpr &fn) const {
        std::print("fn (");
        for (auto pair : fn.arguments) {
            std::print("{}: ", pair.first);
            if (pair.second)
                std::visit(PrintType{}, pair.second.value());
            else
                std::print("<unknown type>");
        }
        std::print(") -> ");
        if (fn.returns)
            std::visit(PrintType{}, fn.returns.value());
        else
            std::print("<unknown type>");
        std::print(" {{ ");
        std::visit(*this, fn.body->kind);
        std::print(" }}");
    }

    void operator()(const BinaryOperatorExpr &binop) const {
        std::visit(*this, binop.left->kind);
        std::print(" {} ", op_to_str(binop.op));
        std::visit(*this, binop.right->kind);
    }

    void operator()(const LetExpr &vdecl) const {
        std::print("variable {} = ", vdecl.name);
        std::visit(*this, vdecl.value->kind);
    }

    void operator()(const LetRefExpr &vref) const {
        std::print("{}", vref.name);
    }

    void operator()(const ImportExpr &imp) const {
        std::print("import {}", imp.path);
    }

    void operator()(const ReturnExpr &ret) const {
        std::print("return ");
        std::visit(*this, ret.returns->kind);
    }

    void operator()(const SequenceExpr &seq) const {
        for (const Expr &e : seq.sequence) {
            std::visit(*this, e.kind);
            std::println();
        }
    }
};

} // namespace ast
