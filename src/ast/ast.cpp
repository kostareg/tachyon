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

struct Printer {
    void operator()(const LiteralExpr &literal) const {
        std::visit(PrintLiteral{}, literal.value);
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
        std::print("import {}", imp.name);
    }

    void operator()(const SequenceExpr &seq) const {
        for (const Expr &e : seq.sequence) {
            std::visit(*this, e.kind);
        }
    }
};

std::expected<vm::Proto, Error> generate_proto(ExprRefs) {
    return std::unexpected(Error(ErrorKind::ParseError, "A", 0, 0, 0, 0));
}
} // namespace ast
