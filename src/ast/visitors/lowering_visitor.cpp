#include "ast/visitors/lowering_visitor.hpp"
#include "ast/ast.hpp"
#include "ir/ir.hpp"

namespace ast {
void LoweringVisitor::visit(const NumberNode &node) {
    tmp = std::make_unique<ir::NumberNode>(node.value);
}

void LoweringVisitor::visit(const BinaryOperatorNode &node) {
    node.lhs->accept(*this);
    auto left = std::move(tmp);
    node.rhs->accept(*this);
    auto right = std::move(tmp);

    auto tx = tmpVar();

    loweredNodes.push_back(std::make_unique<ir::VariableDeclNode>(
        tx, std::make_unique<ir::BinaryOperatorNode>(node.op, std::move(left),
                                                     std::move(right))));
    tmp = std::make_unique<ir::VariableRefNode>(tx);
}

void LoweringVisitor::visit(const VariableDeclNode &node) {
    node.decl->accept(*this);
    spdlog::trace("ast variable decl node");
    loweredNodes.push_back(
        std::make_unique<ir::VariableDeclNode>(node.name, std::move(tmp)));
}

void LoweringVisitor::visit(const VariableRefNode &node) {
    tmp = std::make_unique<ir::VariableRefNode>(node.name);
}

void LoweringVisitor::visit(const FunctionDefNode &node) {
    // clear the lowered nodes list, copy everything into the function node,
    // then bring back lowered nodes list.
    auto otherNodes = std::move(loweredNodes);
    node.body->accept(*this);
    otherNodes.push_back(std::make_unique<ir::FunctionDefNode>(
        node.name, node.args,
        std::make_unique<ir::SequenceNode>(std::move(loweredNodes))));
    loweredNodes = std::move(otherNodes);
}

void LoweringVisitor::visit(const FunctionCallNode &node) {
    int i = 0;
    std::for_each(node.args.begin(), node.args.end(),
                  [this, i](const std::unique_ptr<ASTNode> &arg) mutable {
                      arg->accept(*this);
                      if (tmp)
                          loweredNodes.push_back(
                              std::make_unique<ir::ParamNode>(
                                  std::move(tmp), "p" + std::to_string(i), i));
                      ++i;
                  });

    auto tx = tmpVar();

    loweredNodes.push_back(
        std::make_unique<ir::FunctionCallNode>(node.name, tx));
    tmp = std::make_unique<ir::VariableRefNode>(tx);
}

void LoweringVisitor::visit(const SequenceNode &node) {
    std::for_each(node.stmts.begin(), node.stmts.end(),
                  [this](const std::unique_ptr<ASTNode> &stmt) {
                      spdlog::trace("ast sequence node");
                      stmt->accept(*this);
                      if (tmp)
                          loweredNodes.push_back(std::move(tmp));
                  });
}
} // namespace ast
