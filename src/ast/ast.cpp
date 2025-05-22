#include <expected>
#include <print>

#include "ast/ast.hpp"

namespace ast {

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

std::expected<vm::Proto, Error> generate_proto(Exprs) {
    return std::unexpected(Error(ErrorKind::ParseError, "A", 0, 0, 0, 0));
}
} // namespace ast
