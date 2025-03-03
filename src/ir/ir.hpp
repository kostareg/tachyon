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

    explicit BinaryOperatorNode(Op op, std::unique_ptr<IRNode> lhs, std::unique_ptr<IRNode> rhs)
        : op(op), lhs(std::move(lhs)), rhs(std::move(rhs)) {};

    // TODO: consider checking in constructor that conditions above are
    //       satisfied.

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

    explicit SequenceNode(std::vector<std::unique_ptr<IRNode>> &&body) : body(std::move(body)) {};

    void accept(Visitor &visitor) override { visitor.visit(*this); }
};

// TODO: will we use this for loops, or can I rename to Function &
//       FunctionCall? Else, it's the same as `SequenceNode`.

/**
 * @brief Block definition.
 */
class BlockNode : public IRNode {
  public:
    std::string label;
    std::unique_ptr<IRNode> body;

    explicit BlockNode(std::string label, std::unique_ptr<IRNode> body)
        : label(label), body(std::move(body)) {};

    void accept(Visitor &visitor) override { visitor.visit(*this); }
};

/**
 * @brief Block call.
 */
class BlockCallNode : public IRNode {
  public:
    std::string label;
    std::string outName;

    explicit BlockCallNode(std::string label, std::string outName)
        : label(label), outName(outName) {};

    void accept(Visitor &visitor) override { visitor.visit(*this); }
};

/**
 * @brief Push parameter to stack.
 */
class ParamNode : public IRNode {
  public:
    std::unique_ptr<IRNode> param;

    explicit ParamNode(std::unique_ptr<IRNode> param) : param(std::move(param)) {};

    void accept(Visitor &visitor) override { visitor.visit(*this); }
};
} // namespace ir
