#include <iostream>

#include "ast/ast.hpp"
#include "ast/visitors/print_visitor.hpp"

namespace ast {
void PrintVisitor::visit(const NumberNode &node) { std::cout << node.value; }

void PrintVisitor::visit(const BinaryOperatorNode &node) {
    std::cout << "(";
    node.lhs->accept(*this);
    std::cout << " " << op_to_str(node.op) << " ";
    node.rhs->accept(*this);
    std::cout << ")";
}

void PrintVisitor::visit(const VariableDeclNode &node) {
    std::cout << node.name << " = ";
    node.decl->accept(*this);
}

void PrintVisitor::visit(const VariableRefNode &node) {
    std::cout << node.name;
}

void PrintVisitor::visit(const FunctionDefNode &node) {
    std::cout << "def " << node.name << "(";
    std::for_each(node.args.begin(), node.args.end(),
                  [](const std::string &arg) { std::cout << arg << ","; });
    std::cout << ") {";
    node.body->accept(*this);
    std::cout << "}";
}

void PrintVisitor::visit(const FunctionCallNode &node) {
    std::cout << node.name << "(";
    std::for_each(node.args.begin(), node.args.end(),
                  [this](const std::unique_ptr<ASTNode> &arg) {
                      if (arg)
                          arg->accept(*this);
                      std::cout << ",";
                  });
    std::cout << ")";
}

void PrintVisitor::visit(const SequenceNode &node) {
    std::for_each(node.stmts.begin(), node.stmts.end(),
                  [this](const std::unique_ptr<ASTNode> &stmt) {
                      if (stmt)
                          stmt->accept(*this);
                      std::cout << ";" << std::endl;
                  });
}
} // namespace ast
