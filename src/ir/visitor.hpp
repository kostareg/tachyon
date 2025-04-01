#pragma once

#include <cstdint>
#include <stdexcept>
#include <unordered_map>

#include "op.hpp"
#include "vm/vm.hpp"

namespace ir {
// fwd-decls of ir.hpp nodes.
class IRNode;
class NumberNode;
class VariableRefNode;
class VariableDeclNode;
class BinaryOperatorNode;
class SequenceNode;
class FunctionDefNode;
class FunctionCallNode;
class ParamNode;

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
    virtual void visit(const FunctionDefNode &node) = 0;
    virtual void visit(const FunctionCallNode &node) = 0;
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
    void visit(const FunctionDefNode &node) override;
    void visit(const FunctionCallNode &node) override;
    void visit(const ParamNode &node) override;
};

/**
 * @brief Lower to bytecode.
 * @sa VM::run()
 *
 * Creates a "main" vm::Proto node that stores the bytecode of the body of the
 * AST, and propagates lowering of child functions to their own visitors. At
 * this point, the AST has already checked that all called functions are
 * defined and *in scope*. This assumption simplifies our work here.
 */
class LoweringVisitor : public Visitor {
  public:
    uint16_t program[0xFFF8]; // without headers
    std::string vars[64];     // var names for registers
    uint16_t pc = 0;
    uint16_t pcDef = 33793; // pc for definitions section
    uint16_t tmp;
    std::unordered_map<std::string, int> blocks;

    std::unique_ptr<vm::Proto> proto;

    LoweringVisitor(std::string name)
        : proto(std::make_unique<vm::Proto>(name, nullptr)) {}

    void visit(const NumberNode &node) override;
    void visit(const VariableRefNode &node) override;
    void visit(const VariableDeclNode &node) override;
    void visit(const BinaryOperatorNode &node) override;
    void visit(const SequenceNode &node) override;
    void visit(const FunctionDefNode &node) override;
    void visit(const FunctionCallNode &node) override;
    void visit(const ParamNode &node) override;

    void finish() { proto->bc = program; }

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
            throw std::runtime_error(
                "trying to find the bytecode of unknown operator");
        }
    }
};

} // namespace ir
