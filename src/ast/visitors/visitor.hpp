#pragma once

namespace ast {
// fwd-decls of ast.hpp nodes.
class NumberNode;
class BinaryOperatorNode;
class VariableDeclNode;
class VariableRefNode;
class FunctionDefNode;
class FunctionCallNode;
class SequenceNode;

/**
 * @brief Visitor pattern for abstract syntax tree.
 * @sa ast::ASTNode
 * @sa ast::ASTNode::accept()
 *
 * Derive this class in order to use `ASTNode::accept(Visitor &visitor)`.
 * Each `Visitor::visit` implementation receives immutable access to the node.
 */
class Visitor {
  public:
    virtual void visit(const NumberNode &node) = 0;
    virtual void visit(const BinaryOperatorNode &node) = 0;
    virtual void visit(const VariableDeclNode &node) = 0;
    virtual void visit(const VariableRefNode &node) = 0;
    virtual void visit(const FunctionDefNode &node) = 0;
    virtual void visit(const FunctionCallNode &node) = 0;
    virtual void visit(const SequenceNode &node) = 0;
    virtual ~Visitor() = default;
};
} // namespace ast
