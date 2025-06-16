#pragma once

#include "ast.h"

#include <string>
#include <variant>

namespace tachyon::parser
{
// TODO: consider using just ostream <<.
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

struct PrintExpr
{
    void operator()(const LiteralExpr &literal) const;
    void operator()(const FnExpr &fn) const;
    void operator()(const UnaryOperatorExpr &unop) const;
    void operator()(const BinaryOperatorExpr &binop) const;
    void operator()(const LetExpr &vdecl) const;
    void operator()(const LetRefExpr &vref) const;
    void operator()(const FnCallExpr &fnc) const;
    void operator()(const WhileLoopExpr &wlop) const;
    void operator()(const BreakExpr &brk) const;
    void operator()(const ContinueExpr &) const;
    void operator()(const ImportExpr &imp) const;
    void operator()(const ReturnExpr &ret) const;
    void operator()(const SequenceExpr &seq) const;
};

/**
 * @brief print an expression
 * @param e expression
 */
inline void printExpr(Expr &e)
{
    PrintExpr printer{};
    std::visit(printer, e.kind);
}
} // namespace tachyon::parser