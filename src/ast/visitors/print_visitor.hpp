#pragma once

#include "ast/visitors/visitor.hpp"

namespace ast {
/**
 * @brief Formatted print.
 */
class PrintVisitor : public Visitor {
  public:
    void visit(const NumberNode &node) override;
    void visit(const BinaryOperatorNode &node) override;
    void visit(const VariableDeclNode &node) override;
    void visit(const VariableRefNode &node) override;
    void visit(const FunctionDefNode &node) override;
    void visit(const FunctionCallNode &node) override;
    void visit(const SequenceNode &node) override;
};
} // namespace ast
