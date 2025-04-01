#pragma once

#include <memory>
#include <string>
#include <vector>

#include "ir/visitor.hpp"
#include "op.hpp"

/**
 * @namespace ir
 * @brief High-level intermediate representation nodes.
 */
namespace ir {
/**
 * @brief Base class for intermediate representation nodes.
 *
 * This is a high-level intermediate representation (HIR) that uses
 * three-address code (TAC).
 */
class IRNode {
  public:
    /**
     * @brief Accepts a visitor.
     * @param visitor The visitor.
     */
    virtual void accept(Visitor &visitor) = 0;
    virtual ~IRNode() = default;
};

/**
 * @brief Integer number.
 */
class NumberNode : public IRNode {
  public:
    int value;

    explicit NumberNode(int val) : value(val) {}

    void accept(Visitor &visitor) override { visitor.visit(*this); }
};

/**
 * @brief Binary operator.
 * @sa ::Op
 *
 * Since we are using three-address code and constant folding was done in the
 * first AST optimization run, this will either be:
 *   1. const <op> var
 *   2. var <op> const
 *   3. var <op> var
 */
class BinaryOperatorNode : public IRNode {
  public:
    Op op;
    std::unique_ptr<IRNode> lhs;
    std::unique_ptr<IRNode> rhs;

    explicit BinaryOperatorNode(Op op, std::unique_ptr<IRNode> lhs,
                                std::unique_ptr<IRNode> rhs)
        : op(op), lhs(std::move(lhs)), rhs(std::move(rhs)) {};

    void accept(Visitor &visitor) override { visitor.visit(*this); }
};

/**
 * @brief Variable declaration.
 */
class VariableDeclNode : public IRNode {
  public:
    std::string name;
    std::unique_ptr<IRNode> decl;

    explicit VariableDeclNode(std::string name, std::unique_ptr<IRNode> decl)
        : name(std::move(name)), decl(std::move(decl)) {}

    void accept(Visitor &visitor) override { visitor.visit(*this); }
};

/**
 * @brief Variable reference.
 */
class VariableRefNode : public IRNode {
  public:
    std::string name;

    explicit VariableRefNode(std::string name) : name(std::move(name)) {}

    void accept(Visitor &visitor) override { visitor.visit(*this); }
};

/**
 * @brief Sequence of IRNodes.
 * @sa ir::IRNode
 *
 * It's just a vector of pointers to IRNodes, enabling their sequential
 * composition.
 */
class SequenceNode : public IRNode {
  public:
    std::vector<std::unique_ptr<IRNode>> body;

    explicit SequenceNode(std::vector<std::unique_ptr<IRNode>> &&body)
        : body(std::move(body)) {};

    void accept(Visitor &visitor) override { visitor.visit(*this); }
};

/**
 * @brief Block definition.
 */
class FunctionDefNode : public IRNode {
  public:
    std::string label;
    std::vector<std::string> params;
    std::unique_ptr<IRNode> body;

    explicit FunctionDefNode(std::string label, std::vector<std::string> params,
                             std::unique_ptr<IRNode> body)
        : label(label), params(std::move(params)), body(std::move(body)) {};

    void accept(Visitor &visitor) override { visitor.visit(*this); }
};

/**
 * @brief Block call.
 */
class FunctionCallNode : public IRNode {
  public:
    std::string label;
    std::vector<std::string> params;
    std::string outName;

    explicit FunctionCallNode(std::string label, std::string outName)
        : label(label), outName(outName) {};

    void accept(Visitor &visitor) override { visitor.visit(*this); }
};

/**
 * @brief Push parameter to stack.
 */
class ParamNode : public IRNode {
  public:
    std::unique_ptr<IRNode> param;
    std::string name;
    int idx;

    explicit ParamNode(std::unique_ptr<IRNode> param, std::string name, int idx)
        : param(std::move(param)), name(name), idx(idx) {};

    void accept(Visitor &visitor) override { visitor.visit(*this); }
};
} // namespace ir
