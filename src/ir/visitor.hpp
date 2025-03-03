#pragma once

#include "op.hpp"
#include <memory>
#include <unordered_map>

namespace ir {
class IRNode;
class NumberNode;
class VariableRefNode;
class VariableDeclNode;
class BinaryOperatorNode;
class SequenceNode;
class BlockNode;
class BlockCallNode;
class ParamNode;
class PrintNode;

class Visitor {
  public:
    // TODO: cant return visitor types in ir.hpp?
    virtual std::unique_ptr<IRNode> visit(NumberNode &node) = 0;
    virtual std::unique_ptr<IRNode> visit(VariableRefNode &node) = 0;
    virtual std::unique_ptr<IRNode> visit(VariableDeclNode &node) = 0;
    virtual std::unique_ptr<IRNode> visit(BinaryOperatorNode &node) = 0;
    virtual std::unique_ptr<IRNode> visit(SequenceNode &node) = 0;
    virtual std::unique_ptr<IRNode> visit(BlockNode &node) = 0;
    virtual std::unique_ptr<IRNode> visit(BlockCallNode &node) = 0;
    virtual std::unique_ptr<IRNode> visit(ParamNode &node) = 0;
    virtual ~Visitor() = default;
};

class PrintVisitor : public Visitor {
  public:
    std::string prefix;
    std::unique_ptr<IRNode> visit(NumberNode &node) override;
    std::unique_ptr<IRNode> visit(VariableRefNode &node) override;
    std::unique_ptr<IRNode> visit(VariableDeclNode &node) override;
    std::unique_ptr<IRNode> visit(BinaryOperatorNode &node) override;
    std::unique_ptr<IRNode> visit(SequenceNode &node) override;
    std::unique_ptr<IRNode> visit(BlockNode &node) override;
    std::unique_ptr<IRNode> visit(BlockCallNode &node) override;
    std::unique_ptr<IRNode> visit(ParamNode &node) override;
};

class LoweringVisitor : public Visitor {
  public:
    uint16_t program[0x0FFF];
    std::string vars[32]; // var names for registers
    uint16_t pc;
    uint16_t tmp[0xFF];
    std::unordered_map<std::string, int> blocks;

    std::unique_ptr<IRNode> visit(NumberNode &node) override;
    std::unique_ptr<IRNode> visit(VariableRefNode &node) override;
    std::unique_ptr<IRNode> visit(VariableDeclNode &node) override;
    std::unique_ptr<IRNode> visit(BinaryOperatorNode &node) override;
    std::unique_ptr<IRNode> visit(SequenceNode &node) override;
    std::unique_ptr<IRNode> visit(BlockNode &node) override;
    std::unique_ptr<IRNode> visit(BlockCallNode &node) override;
    std::unique_ptr<IRNode> visit(ParamNode &node) override;
    /// Find an unused register. Returns -1 if all are occupied.
    int findFreeReg() {
        for (size_t i = 0; i < 32; ++i) { // # of registers here.
            if (vars[i].empty()) {
                return i;
            }
        }
        return -1;
    }
    /// Find a variable in the var names, returns the register or -1 if not
    /// found.
    int findVar(std::string name) {
        for (int i = 0; i < 32; ++i) { // # of registers here
            if (vars[i] == name) {
                return i;
            }
        }
        return -1;
    }
    /// Given an operator Op, find the corresponding bytecode value.
    int makeOperator(::Op op) {
        switch (op) {
        case Op::Add:
            return 13;
        case Op::Sub:
            return 14;
        case Op::Mul:
            return 15;
        case Op::Div:
            return 16;
        case Op::Pow:
            return 17;
        }
    }
};

} // namespace ir
