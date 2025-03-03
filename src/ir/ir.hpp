#pragma once

#include "../op.hpp"
#include "visitor.hpp"

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace ir {
/// Base class for IR (HIR) nodes. TAC representation.
class IRNode {
  public:
    virtual std::unique_ptr<IRNode> accept(Visitor &visitor) = 0;
    virtual ~IRNode() = default;
};

class NumberNode : public IRNode {
  public:
    int value;

    explicit NumberNode(int val) : value(val) {}

    std::unique_ptr<IRNode> accept(Visitor &visitor) override { return visitor.visit(*this); }
};

class VariableRefNode : public IRNode {
  public:
    std::string name;

    VariableRefNode(std::string name) : name(std::move(name)) {}

    std::unique_ptr<IRNode> accept(Visitor &visitor) override { return visitor.visit(*this); }
};

class VariableDeclNode : public IRNode {
  public:
    std::string name;
    std::unique_ptr<IRNode> decl;

    VariableDeclNode(std::string name, std::unique_ptr<IRNode> decl)
        : name(std::move(name)), decl(std::move(decl)) {}

    std::unique_ptr<IRNode> accept(Visitor &visitor) override { return visitor.visit(*this); }
};

class BinaryOperatorNode : public IRNode {
  public:
    Op op;
    std::unique_ptr<IRNode> lhs;
    std::unique_ptr<IRNode> rhs;

    BinaryOperatorNode(Op op, std::unique_ptr<IRNode> lhs, std::unique_ptr<IRNode> rhs)
        : op(op), lhs(std::move(lhs)), rhs(std::move(rhs)) {};

    std::unique_ptr<IRNode> accept(Visitor &visitor) override { return visitor.visit(*this); }
};

class SequenceNode : public IRNode {
  public:
    std::vector<std::unique_ptr<IRNode>> body;

    SequenceNode(std::vector<std::unique_ptr<IRNode>> &&body) : body(std::move(body)) {};

    std::unique_ptr<IRNode> accept(Visitor &visitor) override { return visitor.visit(*this); }
};

class BlockNode : public IRNode {
  public:
    std::string label;
    std::unique_ptr<IRNode> body;

    BlockNode(std::string label, std::unique_ptr<IRNode> body)
        : label(label), body(std::move(body)) {};

    std::unique_ptr<IRNode> accept(Visitor &visitor) override { return visitor.visit(*this); }
};

class BlockCallNode : public IRNode {
  public:
    std::string label;
    std::string outName;

    BlockCallNode(std::string label, std::string outName) : label(label), outName(outName) {};

    std::unique_ptr<IRNode> accept(Visitor &visitor) override { return visitor.visit(*this); }
};

class ParamNode : public IRNode {
  public:
    std::unique_ptr<IRNode> param;

    ParamNode(std::unique_ptr<IRNode> param) : param(std::move(param)) {};

    std::unique_ptr<IRNode> accept(Visitor &visitor) override { return visitor.visit(*this); }
};
} // namespace ir
