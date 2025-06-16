#include "tachyon/parser/print.h"

#include <ranges>

namespace tachyon::parser
{
void PrintLiteral::operator()(const std::monostate _) const
{
    std::print("()");
};

void PrintLiteral::operator()(const double dbl) const
{
    std::print("{}", dbl);
};

void PrintLiteral::operator()(const std::string &str) const
{
    std::print("{}", str);
};

void PrintLiteral::operator()(const bool bl) const
{
    std::print("{}", bl ? "true" : "false");
};

void PrintType::operator()(const BasicConcreteTypes &btyp) const
{
    if (btyp == BasicConcreteTypes::Number)
        std::print("Num");
    else if (btyp == BasicConcreteTypes::String)
        std::print("Str");
    else if (btyp == BasicConcreteTypes::Boolean)
        std::print("Bool");
    else if (btyp == BasicConcreteTypes::Unit)
        std::print("()");
};

void PrintType::operator()(const FunctionConcreteTypes &ftyp) const
{
    for (auto val : ftyp.arguments | std::views::values)
    {
        std::visit(*this, val);
    }
};

void PrintType::operator()(const std::string &otyp) const
{
    std::print("{}", otyp);
};

void PrintExpr::operator()(const LiteralExpr &lit) const
{
    std::visit(PrintLiteral{}, lit.value);
};

void PrintExpr::operator()(const FnExpr &fn) const
{
    std::print("fn (");
    for (auto [fst, snd] : fn.arguments)
    {
        std::print("{}: ", fst);
        if (snd)
            std::visit(PrintType{}, snd.value());
        else
            std::print("<unknown type>");
    }
    std::print(") -> ");
    if (fn.returns)
        std::visit(PrintType{}, fn.returns.value());
    else
        std::print("<unknown type>");
    std::print(" {{\n");
    std::visit(*this, fn.body->kind);
    std::print("}}");
};

void PrintExpr::operator()(const UnaryOperatorExpr &unop) const
{
    std::print("({} ", opToStr(unop.op));
    std::visit(*this, unop.right->kind);
    std::print(")");
}

void PrintExpr::operator()(const BinaryOperatorExpr &binop) const
{
    std::print("{} (", opToStr(binop.op));
    std::visit(*this, binop.left->kind);
    std::print(", ");
    std::visit(*this, binop.right->kind);
    std::print(")");
};

void PrintExpr::operator()(const LetExpr &vdecl) const
{
    std::print("variable {} = ", vdecl.name);
    std::visit(*this, vdecl.value->kind);
}

void PrintExpr::operator()(const LetRefExpr &vref) const
{
    std::print("{}", vref.name);
};

void PrintExpr::operator()(const FnCallExpr &fnc) const
{
    std::print("call {} with: ", fnc.ref.name);
    if (fnc.args.empty())
        std::print("<empty>");
    else
        for (const Expr &arg : fnc.args)
        {
            std::visit(*this, arg.kind);
            std::print(", ");
        }
};

void PrintExpr::operator()(const WhileLoopExpr &wlop) const
{
    std::print("while (");
    std::visit(*this, wlop.condition->kind);
    std::print(") {{ ");
    std::visit(*this, wlop.body->kind);
    std::print("}}");
};

void PrintExpr::operator()(const BreakExpr &brk) const
{
    std::print("break ");
    std::visit(*this, brk.returns->kind);
};

void PrintExpr::operator()(const ContinueExpr &) const
{
    std::print("continue");
};

void PrintExpr::operator()(const ImportExpr &imp) const
{
    std::print("import {}", imp.path);
};

void PrintExpr::operator()(const ReturnExpr &ret) const
{
    std::print("return ");
    std::visit(*this, ret.returns->kind);
};

void PrintExpr::operator()(const SequenceExpr &seq) const
{
    for (const Expr &e : seq.sequence)
    {
        std::print("seq> ");
        std::visit(*this, e.kind);
        std::println();
    }
};
} // namespace tachyon::parser