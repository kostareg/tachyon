module;

#include <expected>
#include <print>
#include <variant>

module ast;

namespace ast {
std::expected<vm::Proto, Error> generate_proto(Expr) {
    return std::unexpected(Error(ErrorKind::ParseError, "Z", 0, 0, 0, 0));
}

void PrintLiteral::operator()(const std::monostate _) const {
    std::print("()");
};

void PrintLiteral::operator()(const double dbl) const {
    std::print("{}", dbl);
};

void PrintLiteral::operator()(const std::string &str) const {
    std::print("{}", str);
};

void PrintLiteral::operator()(const bool bl) const {
    std::print("{}", bl ? "true" : "false");
};

void PrintType::operator()(const BasicConcreteTypes &btyp) const {
    if (btyp == BasicConcreteTypes::Number)
        std::print("Num");
    else if (btyp == BasicConcreteTypes::String)
        std::print("Str");
    else if (btyp == BasicConcreteTypes::Boolean)
        std::print("Bool");
    else if (btyp == BasicConcreteTypes::Unit)
        std::print("()");
};

void PrintType::operator()(const FunctionConcreteTypes &ftyp) const {
    for (auto pair : ftyp.arguments) {
        std::visit(*this, pair.second);
    }
};

void PrintType::operator()(const std::string &otyp) const {
    std::print("{}", otyp);
};

void Printer::operator()(const LiteralExpr &literal) const {
    std::visit(PrintLiteral{}, literal.value);
};

void Printer::operator()(const FnExpr &fn) const {
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
    std::print(" {{\n");
    std::visit(*this, fn.body->kind);
    std::print("}}");
};

void Printer::operator()(const BinaryOperatorExpr &binop) const {
    std::print("{} (", op_to_str(binop.op));
    std::visit(*this, binop.left->kind);
    std::print(", ");
    std::visit(*this, binop.right->kind);
    std::print(")");
};

void Printer::operator()(const LetExpr &vdecl) const {
    std::print("variable {} = ", vdecl.name);
    std::visit(*this, vdecl.value->kind);
}

void Printer::operator()(const LetRefExpr &vref) const {
    std::print("{}", vref.name);
};

void Printer::operator()(const FnCallExpr &fnc) const {
    std::print("call {} with: ", fnc.ref.name);
    if (fnc.args.empty())
        std::print("<empty>");
    else
        for (const auto &arg : fnc.args) {
            std::visit(*this, arg.kind);
            std::print(", ");
        }
};

void Printer::operator()(const ImportExpr &imp) const {
    std::print("import {}", imp.path);
};

void Printer::operator()(const ReturnExpr &ret) const {
    std::print("return ");
    std::visit(*this, ret.returns->kind);
};

void Printer::operator()(const SequenceExpr &seq) const {
    for (const Expr &e : seq.sequence) {
        std::visit(*this, e.kind);
        std::println();
    }
};

void BytecodeGenerator::operator()(const LiteralExpr &literal) {};
void BytecodeGenerator::operator()(const FnExpr &fn) {};
void BytecodeGenerator::operator()(const BinaryOperatorExpr &binop) {};
void BytecodeGenerator::operator()(const LetExpr &vdecl) {};
void BytecodeGenerator::operator()(const LetRefExpr &vref) {};
void BytecodeGenerator::operator()(const FnCallExpr &fnc) {};
void BytecodeGenerator::operator()(const ImportExpr &imp) {};
void BytecodeGenerator::operator()(const ReturnExpr &ret) {};
void BytecodeGenerator::operator()(const SequenceExpr &seq) {};

} // namespace ast
