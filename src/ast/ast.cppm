module;

#include <expected>
#include <memory>
#include <print>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

export module ast;

import error;
import op;
import vm;

/**
 * @namespace ast
 * @brief abstract syntax tree nodes
 */
namespace ast {
// fwd-decl
export struct Expr;
export using ExprRef = std::unique_ptr<Expr>;
export using Exprs = std::vector<Expr>;

/**
 * @brief literal values, ie unit, numbers, strings, booleans
 */
export using LiteralValue =
    std::variant<std::monostate, double, std::string, bool>;

export struct LiteralExpr {
    LiteralValue value;
};

/**
 * @brief four basic concrete types.
 */
export enum class BasicConcreteTypes {
    Number,
    String,
    Boolean,
    Unit,
};

// fwd-decl
export struct FunctionConcreteTypes;

/**
 * @brief the "Type" value, ie a reference to a concrete type or enum/struct
 */
export using Type =
    std::variant<BasicConcreteTypes, FunctionConcreteTypes, std::string>;

export using MaybeType = std::optional<Type>;

export struct FunctionConcreteTypes {
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
export struct FnExpr {
    std::unordered_map<std::string, MaybeType> arguments;
    MaybeType returns;
    ExprRef body;
};

/**
 * @brief <expr> +, -, *, /, ^ <expr>
 */
export struct BinaryOperatorExpr {
    Op op;
    ExprRef left;
    ExprRef right;
};

export struct LetExpr {
    std::string name;
    ExprRef value;
};

export struct LetRefExpr {
    std::string name;
};

// TODO: captures
export struct FnCallExpr {
    LetRefExpr ref;
    std::vector<Expr> args;
};

export struct ImportExpr {
    std::string path;
};

export struct ReturnExpr {
    ExprRef returns;
};

export struct SequenceExpr {
    Exprs sequence;
};

export using ExprKind =
    std::variant<LiteralExpr, FnExpr, BinaryOperatorExpr, LetExpr, LetRefExpr,
                 FnCallExpr, ImportExpr, ReturnExpr, SequenceExpr>;

/**
 * @brief Top-level expression type.
 */
export struct Expr {
    ExprKind kind;
    SourceSpan span;
};

export std::expected<vm::Proto, Error> generate_proto(Expr);

// visitors
struct PrintLiteral {
    void operator()(std::monostate _) const;
    void operator()(double dbl) const;
    void operator()(const std::string &str) const;
    void operator()(bool bl) const;
};

struct PrintType {
    void operator()(const BasicConcreteTypes &btyp) const;
    void operator()(const FunctionConcreteTypes &ftyp) const;
    void operator()(const std::string &otyp) const;
};

export struct Printer {
    void operator()(const LiteralExpr &literal) const;
    void operator()(const FnExpr &fn) const;
    void operator()(const BinaryOperatorExpr &binop) const;
    void operator()(const LetExpr &vdecl) const;
    void operator()(const LetRefExpr &vref) const;
    void operator()(const FnCallExpr &fnc) const;
    void operator()(const ImportExpr &imp) const;
    void operator()(const ReturnExpr &ret) const;
    void operator()(const SequenceExpr &seq) const;
};

export std::expected<Expr, Error> print(Expr e) {
    auto printer = Printer{};
    std::visit(printer, e.kind);
    return e;
}

export struct TypeInferrer {
    std::vector<Error> errors;
    SourceSpan span;

    TypeInferrer(SourceSpan span) : span(span) {};

    void operator()(LiteralExpr &literal);
    void operator()(FnExpr &fn);
    void operator()(BinaryOperatorExpr &binop);
    void operator()(LetExpr &vdecl);
    void operator()(LetRefExpr &vref);
    void operator()(FnCallExpr &fnc);
    void operator()(ImportExpr &imp);
    void operator()(ReturnExpr &ret);
    void operator()(SequenceExpr &seq);
};

export std::expected<Expr, Error> infer(Expr e) {
    auto inferred = TypeInferrer{e.span};
    std::visit(inferred, e.kind);
    if (!inferred.errors.empty())
        return std::unexpected(Error(inferred.errors));
    return e;
}

export struct BytecodeGenerator {
    void operator()(const LiteralExpr &literal);
    void operator()(const FnExpr &fn);
    void operator()(const BinaryOperatorExpr &binop);
    void operator()(const LetExpr &vdecl);
    void operator()(const LetRefExpr &vref);
    void operator()(const FnCallExpr &fnc);
    void operator()(const ImportExpr &imp);
    void operator()(const ReturnExpr &ret);
    void operator()(const SequenceExpr &seq);
};

} // namespace ast
