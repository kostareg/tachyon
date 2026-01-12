#include "tachyon/parser/print.hpp"

#include <ranges>

namespace tachyon::parser {
void PrintLiteral::operator()(const std::monostate) const {
    std::cout << "()";
};

void PrintLiteral::operator()(const double dbl) const {
    std::cout << dbl;
};

void PrintLiteral::operator()(const std::string &str) const {
    std::cout << str;
};

void PrintLiteral::operator()(const bool bl) const {
    std::cout << std::boolalpha << bl << std::noboolalpha;
};

void PrintLiteral::operator()(const Matrix &m) const {
    for (size_t i = 0; i < m.size(); ++i) {
        std::cout << m(i) << ", ";
    }
}

void PrintType::operator()(const BasicConcreteTypes &btyp) const {
    if (btyp == BasicConcreteTypes::Number) std::cout << "Num";
    else if (btyp == BasicConcreteTypes::String) std::cout << "Str";
    else if (btyp == BasicConcreteTypes::Boolean) std::cout << "Bool";
    else if (btyp == BasicConcreteTypes::Unit) std::cout << "()";
};

void PrintType::operator()(const FunctionConcreteTypes &ftyp) const {
    for (auto val : ftyp.arguments | std::views::values) {
        std::visit(*this, val);
    }
};

void PrintType::operator()(const std::string &otyp) const {
    std::cout << otyp;
};

void PrintExpr::operator()(const LiteralExpr &lit) const {
    std::visit(PrintLiteral{}, lit.value);
};

void PrintExpr::operator()(const FnExpr &fn) const {
    std::cout << "fn (";
    for (auto [fst, snd] : fn.arguments) {
        std::cout << fst << ": ";
        if (snd) std::visit(PrintType{}, snd.value());
        else std::cout << "<unknown type>";
    }
    std::cout << ") -> ";
    if (fn.returns) std::visit(PrintType{}, fn.returns.value());
    else std::cout << "<unknown type>";
    std::cout << "{" << std::endl;
    std::visit(*this, fn.body->kind);
    std::cout << "}";
};

void PrintExpr::operator()(const UnaryOperatorExpr &unop) const {
    std::cout << op_to_str(unop.op) << "(";
    std::visit(*this, unop.right->kind);
    std::cout << ")";
}

void PrintExpr::operator()(const BinaryOperatorExpr &binop) const {
    std::cout << op_to_str(binop.op) << " (";
    std::visit(*this, binop.left->kind);
    std::cout << ", ";
    std::visit(*this, binop.right->kind);
    std::cout << ")";
};

void PrintExpr::operator()(const LetExpr &vdecl) const {
    std::cout << "variable " << vdecl.name << " = ";
    std::visit(*this, vdecl.value->kind);
}

void PrintExpr::operator()(const LetRefExpr &vref) const {
    std::cout << vref.name;
};

void PrintExpr::operator()(const MatrixAssignmentExpr &mass) const {
    std::cout << "matrix " << mass.ref.name << "->(";
    std::visit(*this, mass.row->kind);
    std::cout << ", ";
    std::visit(*this, mass.col->kind);
    std::cout << ") = ";
    std::visit(*this, mass.value->kind);
}

void PrintExpr::operator()(const MatrixRefExpr &mref) const {
    std::cout << "matrix " << mref.ref.name << "->(";
    std::visit(*this, mref.row->kind);
    std::cout << ", ";
    std::visit(*this, mref.col->kind);
    std::cout << ")";
}

void PrintExpr::operator()(const FnCallExpr &fnc) const {
    std::cout << "call " << fnc.ref.name << " with: ";
    if (fnc.args.empty()) std::cout << "<empty>";
    else
        for (const Expr &arg : fnc.args) {
            std::visit(*this, arg.kind);
            std::cout << ", ";
        }
};

void PrintExpr::operator()(const WhileLoopExpr &wlop) const {
    std::cout << "while (";
    std::visit(*this, wlop.condition->kind);
    std::cout << ") { ";
    std::visit(*this, wlop.body->kind);
    std::cout << "}";
};

void PrintExpr::operator()(const BreakExpr &brk) const {
    std::cout << "break ";
    std::visit(*this, brk.returns->kind);
};

void PrintExpr::operator()(const ContinueExpr &) const {
    std::cout << "continue";
};

void PrintExpr::operator()(const IfExpr &iff) const {
    std::cout << "if (";
    std::visit(*this, iff.condition->kind);
    std::cout << ") then (";
    std::visit(*this, iff.body->kind);
    std::cout << ")";
    for (size_t i = 0; i < iff.else_if_conditions.size(); ++i) {
        std::cout << "else if (";
        std::visit(*this, iff.else_if_conditions[i].kind);
        std::cout << ") then (";
        std::visit(*this, iff.else_if_bodies[i].kind);
        std::cout << ")";
    }
    if (iff.else_body != nullptr) {
        std::cout << "else ";
        std::visit(*this, iff.else_body->kind);
    }
}

void PrintExpr::operator()(const ImportExpr &imp) const {
    std::cout << "import " << imp.path;
};

void PrintExpr::operator()(const ReturnExpr &ret) const {
    std::cout << "return ";
    std::visit(*this, ret.returns->kind);
};

void PrintExpr::operator()(const MatrixConstructExpr &mc) const {
    std::cout << "make matrix h=" << mc.height << " elems=";
    for (const Expr &e : mc.list) {
        std::visit(*this, e.kind);
        std::cout << ", ";
    }
}

void PrintExpr::operator()(const SequenceExpr &seq) const {
    for (const Expr &e : seq.sequence) {
        std::cout << "seq> ";
        std::visit(*this, e.kind);
        std::cout << std::endl;
    }
};
} // namespace tachyon::parser