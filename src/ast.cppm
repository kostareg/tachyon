module;

#include <expected>
#include <memory>
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
namespace ast
{
// fwd-decl
export struct Expr;
export using ExprRef = std::unique_ptr<Expr>;
export using Exprs = std::vector<Expr>;

/**
 * @brief literal values, ie unit, numbers, strings, booleans
 */
export using LiteralValue = std::variant<std::monostate, double, std::string, bool>;

export struct LiteralExpr
{
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
export using Type = std::variant<BasicConcreteTypes, FunctionConcreteTypes, std::string>;

export using MaybeType = std::optional<Type>;

export struct FunctionConcreteTypes
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
export struct FnExpr
{
    std::vector<std::pair<std::string, MaybeType>> arguments;
    MaybeType returns;
    ExprRef body;
};

/**
 * @brief <expr> +, -, *, /, ^ <expr>
 */
export struct BinaryOperatorExpr
{
    Op op;
    ExprRef left;
    ExprRef right;
};

export struct LetExpr
{
    std::string name;
    ExprRef value;
};

export struct LetRefExpr
{
    std::string name;
};

// TODO: captures
export struct FnCallExpr
{
    LetRefExpr ref;
    std::vector<Expr> args;
};

export struct ImportExpr
{
    std::string path;
};

// TODO: retv
export struct ReturnExpr
{
    ExprRef returns;
};

export struct SequenceExpr
{
    Exprs sequence;
};

export using ExprKind = std::variant<LiteralExpr, FnExpr, BinaryOperatorExpr, LetExpr, LetRefExpr,
                                     FnCallExpr, ImportExpr, ReturnExpr, SequenceExpr>;

/**
 * @brief Top-level expression type.
 */
export struct Expr
{
    ExprKind kind;
    SourceSpan span;
};

// TODO: type checking, inference. May also use type erasure just before
//  bytecode generation, that way BytecodeGenerator for FnExpr doesn't even know
//  the type of the arguments for example.

// visitors
struct PrintLiteral
{
    void operator()(std::monostate _) const;
    void operator()(double dbl) const;
    void operator()(const std::string &str) const;
    void operator()(bool bl) const;
};

struct PrintType
{
    void operator()(const BasicConcreteTypes &btyp) const;
    void operator()(const FunctionConcreteTypes &ftyp) const;
    void operator()(const std::string &otyp) const;
};

export struct Printer
{
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

export std::expected<Expr, Error> print(Expr e)
{
    auto printer = Printer{};
    std::visit(printer, e.kind);
    return e;
}

export struct BytecodeGenerator
{
    std::vector<uint8_t> bc;
    std::vector<vm::Value> constants;
    // TODO: is this the best way to do this? consider how it is finding vars by
    //  name. also, since its unique consider std::set.
    std::vector<std::string> vars;
    // TODO: this certainly does not work because even if we set the value
    //  during construction, it still won't be valid if you declare a new
    //  variable and then use it (will point to newest var). and not the
    //  simplest fix because what if you use it, *then* create the new vars,
    //  then use it again?
    size_t next_free_register = 1;
    uint8_t curr;
    std::vector<Error> errors;

    BytecodeGenerator() {};
    explicit BytecodeGenerator(std::vector<std::string> vars) : vars(std::move(vars))
    {
        next_free_register = vars.size() + 1;
    };

    void operator()(const LiteralExpr &lit);
    void operator()(const FnExpr &fn);
    void operator()(const BinaryOperatorExpr &binop);
    void operator()(const LetExpr &vdecl);
    void operator()(const LetRefExpr &vref);
    void operator()(const FnCallExpr &fnc);
    void operator()(const ImportExpr &imp);
    void operator()(const ReturnExpr &ret);
    void operator()(const SequenceExpr &seq);
};

export std::expected<vm::Proto, Error> generateProto(Expr e)
{
    auto generator = BytecodeGenerator{};
    std::visit(generator, e.kind);
    if (!generator.errors.empty())
        return std::unexpected(Error(generator.errors));
    return vm::Proto(generator.bc, std::move(generator.constants), 0, "<anonymous>", e.span);
}

// TODO: I wanted to do this with function overloading or defaults but the
//  and_then calls were not working. clean this up.
export std::expected<vm::Proto, Error> generateProtoWithArgs(Expr e, std::vector<std::string> args)
{
    auto size = args.size();
    auto generator = BytecodeGenerator(std::move(args));
    std::visit(generator, e.kind);
    if (!generator.errors.empty())
        return std::unexpected(Error(generator.errors));
    return vm::Proto(generator.bc, std::move(generator.constants), size, "<anonymous>", e.span);
}

} // namespace ast
