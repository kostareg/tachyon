#include <algorithm>
#include <iostream>
#include <spdlog/spdlog.h>

#include "ir/ir.hpp"
#include "ir/visitor.hpp"

namespace ir {
void PrintVisitor::visit(const NumberNode &node) { std::cout << node.value; }

void PrintVisitor::visit(const VariableRefNode &node) {
    std::cout << node.name;
}

void PrintVisitor::visit(const VariableDeclNode &node) {
    std::cout << prefix << node.name << " = ";
    node.decl->accept(*this);
}

void PrintVisitor::visit(const BinaryOperatorNode &node) {
    auto prefixO = prefix;
    prefix = "";
    node.lhs->accept(*this);
    std::cout << " " << op_to_str(node.op) << " ";
    node.rhs->accept(*this);
    prefix = prefixO;
}

void PrintVisitor::visit(const SequenceNode &node) {
    std::for_each(node.body.begin(), node.body.end(),
                  [this](const std::unique_ptr<IRNode> &stmt) {
                      stmt->accept(*this);
                      std::cout << std::endl;
                  });
}

void PrintVisitor::visit(const FunctionDefNode &node) {
    std::cout << "LABEL " << node.label << std::endl;

    prefix += "    ";
    if (node.body)
        node.body->accept(*this);
    prefix.erase(prefix.size() - 4);
}

void PrintVisitor::visit(const FunctionCallNode &node) {
    std::cout << prefix << "CALL " << node.label << " TO " << node.outName;
}

void PrintVisitor::visit(const ParamNode &node) {
    std::cout << prefix << "PARAM ";
    node.param->accept(*this);
}

void LoweringVisitor::visit(const NumberNode &node) { tmp = node.value; }

void LoweringVisitor::visit(const VariableRefNode &node) {
    tmp = findVar(node.name);
}

void LoweringVisitor::visit(const VariableDeclNode &node) {
    spdlog::trace("variable declaration");
    if (auto num = dynamic_cast<NumberNode *>(node.decl.get())) {
        // if we are declaring a constant, move it directly.
        auto reg = findFreeReg();

        num->accept(*this);
        program[pc++] = 0x0010;
        program[pc++] = reg;
        program[pc++] = tmp;

        vars[reg] = node.name;
    } else if (auto binop =
                   dynamic_cast<BinaryOperatorNode *>(node.decl.get())) {
        // if we are declaring a binop, let it handle itself and claim its
        // register under our name.
        binop->accept(*this);
        vars[tmp] = node.name;
    } else if (auto var = dynamic_cast<VariableRefNode *>(node.decl.get())) {
        // if we are declaring the same as another variable, copy it.
        var->accept(*this);
        auto to = findFreeReg();

        program[pc++] = 0x0011;
        program[pc++] = to;
        program[pc++] = tmp;

        vars[to] = node.name;
    }
}

void LoweringVisitor::visit(const BinaryOperatorNode &node) {
    if (auto var = dynamic_cast<VariableRefNode *>(node.lhs.get())) {
        if (auto num = dynamic_cast<NumberNode *>(node.rhs.get())) {
            // find the variable's register, then run reg <op> const
            var->accept(*this);
            auto out = findFreeReg();
            program[pc++] = makeOperator(node.op, true, false);
            program[pc++] = tmp;
            program[pc++] = num->value;
            program[pc++] = out;
            tmp = out;
        } else if (auto varR =
                       dynamic_cast<VariableRefNode *>(node.rhs.get())) {
            var->accept(*this);
            auto reg1 = tmp;
            varR->accept(*this);
            auto reg2 = tmp;

            auto out = findFreeReg();
            program[pc++] = makeOperator(node.op, true, true);
            program[pc++] = reg1;
            program[pc++] = reg2;
            program[pc++] = out;
            tmp = out;
        }
    } else if (auto num = dynamic_cast<NumberNode *>(node.lhs.get())) {
        if (auto var = dynamic_cast<VariableRefNode *>(node.rhs.get())) {
            var->accept(*this);
            auto out = findFreeReg();
            program[pc++] = makeOperator(node.op, false, true);
            program[pc++] = num->value;
            program[pc++] = tmp;
            program[pc++] = out;
            tmp = out;
        }
    }
}

void LoweringVisitor::visit(const SequenceNode &node) {
    std::for_each(node.body.begin(), node.body.end(),
                  [this](const std::unique_ptr<IRNode> &elem) {
                      spdlog::trace("sequence node");
                      elem->accept(*this);
                  });
}

void LoweringVisitor::visit(const FunctionDefNode &node) {
    // add it to the children, then hand down handling.
    LoweringVisitor v(node.label);
    // propagate information about variable names, this is handled in order.
    for (size_t i = 0; i < node.params.size(); ++i) {
        v.vars[i] = node.params[i];
    }
    node.body->accept(v);
    proto->children.push_back(std::move(v.proto));
}

void LoweringVisitor::visit(const FunctionCallNode &node) {
    // find where its stored
    if (node.label == "print") {
        // load print somewhere and call it.
        auto reg = findFreeReg();
        program[pc++] = 0x0010;
        program[pc++] = reg;
        program[pc++] = 0xFFFF;
        program[pc++] = 0x0100;
        program[pc++] = reg;
    } else {
        // call position.
        program[pc++] = 0x0100;
        program[pc++] = findVar(node.label);
    }
}

void LoweringVisitor::visit(const ParamNode &node) {
    if (auto num = dynamic_cast<NumberNode *>(node.param.get())) {
        auto reg = findFreeReg();
        num->accept(*this);
        program[pc++] = 0x0110;
        program[pc++] = reg;
        program[pc++] = tmp;

        vars[reg] = node.name;
    } else if (auto var = dynamic_cast<VariableRefNode *>(node.param.get())) {
        var->accept(*this);
        if (tmp != 65535) {
            program[pc++] = 0x0111;
            program[pc++] = node.idx;
            program[pc++] = tmp;
        } else {
            throw std::runtime_error("param uses unknown variable");
        }
    } else {
        throw std::runtime_error("ice: could not handle fn param");
    }
}
} // namespace ir
