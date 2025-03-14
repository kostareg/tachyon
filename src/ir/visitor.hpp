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
    virtual void visit(const PrintNode &node) = 0;
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
    void visit(const PrintNode &node) override;
};

/**
 * @brief Write function block stubs.
 * @sa LoweringVisitor
 *
 * Basically a lightweight first pass of the lowering visitor that records all
 * block names and creates stubs for them. In
 * LoweringVisitor::visit(const BlockNode), stubs are updated to point to the
 * allocated section of memory for the function.
 *
 * Every visitor apart from the BlockNode is placeholder or passthru.
 */
class StubVisitor : public Visitor {
  public:
    uint16_t stubs[0x400];
    uint16_t pc = 0;
    std::unordered_map<std::string, int> blocks;

    void visit(const NumberNode &node) override;
    void visit(const VariableRefNode &node) override;
    void visit(const VariableDeclNode &node) override;
    void visit(const BinaryOperatorNode &node) override;
    void visit(const SequenceNode &node) override;
    void visit(const BlockNode &node) override;
    void visit(const BlockCallNode &node) override;
    void visit(const ParamNode &node) override;
    void visit(const PrintNode &node) override;
};

/**
 * @brief Lower to bytecode.
 * @sa VM::run()
 */
class LoweringVisitor : public Visitor {
  public:
    uint16_t program[0xFFF8]; // without headers
    std::string vars[64];     // var names for registers
    uint16_t pc = 0;
    uint16_t pcDef = 33793; // pc for definitions section
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
    void visit(const PrintNode &node) override;

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
    int makeOperator(::Op op, bool leftVar, bool rightVar) {
        if (!leftVar && !rightVar)
            throw std::runtime_error("ice: cannot interpret const+const");

        switch (op) {
        case Op::Add:
            if (leftVar && rightVar)
                return 0x005A;
            else if (leftVar)
                return 0x0050;
            else
                return 0x0055;
        case Op::Sub:
            if (leftVar && rightVar)
                return 0x005B;
            else if (leftVar)
                return 0x0051;
            else
                return 0x0056;
        case Op::Mul:
            if (leftVar && rightVar)
                return 0x005C;
            else if (leftVar)
                return 0x0052;
            else
                return 0x0057;
        case Op::Div:
            if (leftVar && rightVar)
                return 0x005D;
            else if (leftVar)
                return 0x0053;
            else
                return 0x0058;
        case Op::Pow:
            if (leftVar && rightVar)
                return 0x005E;
            else if (leftVar)
                return 0x0054;
            else
                return 0x0059;
        default:
            throw std::runtime_error("trying to find the bytecode of unknown operator");
        }
    }
};

} // namespace ir
