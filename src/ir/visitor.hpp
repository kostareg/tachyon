#pragma once

#include <cstdint>
#include <stdexcept>
#include <unordered_map>

#include "op.hpp"

namespace ir {
// fwd-decls of ir.hpp nodes.
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

/**
 * @brief Visitor pattern for intermediate representation.
 * @sa ir::IRNode
 * @sa ir::IRNode::accept()
 *
 * Derive this class in order to use `IRNode::accept(Visitor &visitor)`.
 * Each `Visitor::visit` implementation receives immutable access to the node.
 */
class Visitor {
  public:
    virtual void visit(const NumberNode &node) = 0;
    virtual void visit(const VariableRefNode &node) = 0;
    virtual void visit(const VariableDeclNode &node) = 0;
    virtual void visit(const BinaryOperatorNode &node) = 0;
    virtual void visit(const SequenceNode &node) = 0;
    virtual void visit(const BlockNode &node) = 0;
    virtual void visit(const BlockCallNode &node) = 0;
    virtual void visit(const ParamNode &node) = 0;
    virtual ~Visitor() = default;
};

/**
 * @brief Formatted print.
 */
class PrintVisitor : public Visitor {
  public:
    std::string prefix;
    void visit(const NumberNode &node) override;
    void visit(const VariableRefNode &node) override;
    void visit(const VariableDeclNode &node) override;
    void visit(const BinaryOperatorNode &node) override;
    void visit(const SequenceNode &node) override;
    void visit(const BlockNode &node) override;
    void visit(const BlockCallNode &node) override;
    void visit(const ParamNode &node) override;
};

/**
 * @brief Lower to bytecode.
 * @sa VM::run()
 */
class LoweringVisitor : public Visitor {
  public:
    uint16_t program[0x0FFF];
    std::string vars[32]; // var names for registers
    uint16_t pc;
    uint16_t tmp[0xFF];
    std::unordered_map<std::string, int> blocks;

    void visit(const NumberNode &node) override;
    void visit(const VariableRefNode &node) override;
    void visit(const VariableDeclNode &node) override;
    void visit(const BinaryOperatorNode &node) override;
    void visit(const SequenceNode &node) override;
    void visit(const BlockNode &node) override;
    void visit(const BlockCallNode &node) override;
    void visit(const ParamNode &node) override;

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
        default:
            throw std::runtime_error("trying to find the bytecode of unknown operator");
        }
    }
};

} // namespace ir
