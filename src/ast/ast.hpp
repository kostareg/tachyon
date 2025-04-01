#pragma once

#include <memory>
#include <string>
#include <vector>

#include "ast/visitors/visitor.hpp"
#include "op.hpp"

/**
 * @namespace ast
 * @brief Abstract syntax tree nodes.
 */
namespace ast {
/**
 * @brief Base class for all abstract syntax tree nodes.
 *
 * The base class for the abstract syntax tree (AST) outlines the structure
 * that the rest of the nodes will derive.
 */
class ASTNode {
  public:
    /**
     * @brief Accepts a visitor.
     * @param visitor The visitor.
     */
    virtual void accept(Visitor &visitor) = 0;
    virtual ~ASTNode() = default;
};

/**
 * @brief Integer number.
 */
class NumberNode : public ASTNode {
  public:
    int value;

    explicit NumberNode(int val) : value(val) {}

    void accept(Visitor &visitor) override { visitor.visit(*this); }
};

/**
 * @brief Binary operator.
 * @sa ::Op
 */
class BinaryOperatorNode : public ASTNode {
  public:
    ::Op op;
    std::unique_ptr<ASTNode> lhs;
    std::unique_ptr<ASTNode> rhs;

    explicit BinaryOperatorNode(::Op op, std::unique_ptr<ASTNode> left,
                                std::unique_ptr<ASTNode> right)
        : op(op), lhs(std::move(left)), rhs(std::move(right)) {}

    void accept(Visitor &visitor) override { visitor.visit(*this); }
};

/**
 * @brief Variable declaration.
 */
class VariableDeclNode : public ASTNode {
  public:
    std::string name;
    std::unique_ptr<ASTNode> decl;

    explicit VariableDeclNode(std::string name, std::unique_ptr<ASTNode> decl)
        : name(std::move(name)), decl(std::move(decl)) {}

    void accept(Visitor &visitor) override { visitor.visit(*this); }
};

/**
 * @brief Variable reference.
 */
class VariableRefNode : public ASTNode {
  public:
    std::string name;

    explicit VariableRefNode(std::string name) : name(std::move(name)) {}

    void accept(Visitor &visitor) override { visitor.visit(*this); }
};

/**
 * @brief Function definition.
 */
class FunctionDefNode : public ASTNode {
  public:
    std::string name;
    std::vector<std::string> args;
    std::unique_ptr<ASTNode> body;

    explicit FunctionDefNode(std::string name, std::vector<std::string> args,
                             std::unique_ptr<ASTNode> body)
        : name(std::move(name)), args(std::move(args)), body(std::move(body)) {}

    void accept(Visitor &visitor) override { visitor.visit(*this); }
};

/**
 * @brief Function call.
 */
class FunctionCallNode : public ASTNode {
  public:
    std::string name;
    std::vector<std::unique_ptr<ASTNode>> args;

    FunctionCallNode(std::string name,
                     std::vector<std::unique_ptr<ASTNode>> &&args)
        : name(std::move(name)), args(std::move(args)) {}

    void accept(Visitor &visitor) override { visitor.visit(*this); }
};

/**
 * @brief Sequence of ASTNodes.
 * @sa ast::ASTNode
 *
 * It's just a vector of pointers to ASTNodes, enabling their sequential
 * composition.
 */
class SequenceNode : public ASTNode {
  public:
    std::vector<std::unique_ptr<ASTNode>> stmts;

    explicit SequenceNode(std::vector<std::unique_ptr<ASTNode>> &&stmts)
        : stmts(std::move(stmts)) {}

    void accept(Visitor &visitor) override { visitor.visit(*this); }
};
} // namespace ast
