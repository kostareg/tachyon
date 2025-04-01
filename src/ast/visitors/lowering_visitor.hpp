#pragma once

#include "ast/visitors/visitor.hpp"
#include "ir/ir.hpp"

namespace ast {
/**
 * @brief Lower to intermediate representation.
 * @sa ir::IRNode
 */
class LoweringVisitor : public Visitor {
  public:
    std::vector<std::unique_ptr<ir::IRNode>> loweredNodes;
    std::unique_ptr<ir::IRNode> tmp;
    int tmpInt = 0;

    void visit(const NumberNode &node) override;
    void visit(const BinaryOperatorNode &node) override;
    void visit(const VariableDeclNode &node) override;
    void visit(const VariableRefNode &node) override;
    void visit(const FunctionDefNode &node) override;
    void visit(const FunctionCallNode &node) override;
    void visit(const SequenceNode &node) override;

    std::string tmpVar() { return "t" + std::to_string(++tmpInt); }
};
} // namespace ast
