#include "visitor.hpp"
#include "ir.hpp"
#include <algorithm>
#include <iostream>

namespace ir {
std::unique_ptr<IRNode> PrintVisitor::visit(NumberNode &node) {
    std::cout << node.value;
    return nullptr;
}

std::unique_ptr<IRNode> PrintVisitor::visit(VariableRefNode &node) {
    std::cout << node.name;
    return nullptr;
}

std::unique_ptr<IRNode> PrintVisitor::visit(VariableDeclNode &node) {
    std::cout << prefix << node.name << " = ";
    node.decl->accept(*this);
    return nullptr;
}

std::unique_ptr<IRNode> PrintVisitor::visit(BinaryOperatorNode &node) {
    auto prefixO = prefix;
    prefix = "";
    node.lhs->accept(*this);
    std::cout << " " << op_to_string(node.op) << " ";
    node.rhs->accept(*this);
    prefix = prefixO;
    return nullptr;
}

std::unique_ptr<IRNode> PrintVisitor::visit(SequenceNode &node) {
    std::for_each(node.body.begin(), node.body.end(), [this](std::unique_ptr<IRNode> &stmt) {
        stmt->accept(*this);
        std::cout << std::endl;
    });
    return nullptr;
}

std::unique_ptr<IRNode> PrintVisitor::visit(BlockNode &node) {
    std::cout << "LABEL " << node.label << std::endl;

    prefix += "    ";
    if (node.body)
        node.body->accept(*this);
    prefix.erase(prefix.size() - 4);

    return nullptr;
}

std::unique_ptr<IRNode> PrintVisitor::visit(BlockCallNode &node) {
    std::cout << prefix << "CALL " << node.label << " TO " << node.outName;
    return nullptr;
}

std::unique_ptr<IRNode> PrintVisitor::visit(ParamNode &node) {
    std::cout << prefix << "PARAM ";
    node.param->accept(*this);
    return nullptr;
}

std::unique_ptr<IRNode> LoweringVisitor::visit(NumberNode &node) {
    tmp[0] = node.value;
    return nullptr;
}

std::unique_ptr<IRNode> LoweringVisitor::visit(VariableRefNode &node) { return nullptr; }

std::unique_ptr<IRNode> LoweringVisitor::visit(VariableDeclNode &node) {
    if (auto num = dynamic_cast<NumberNode *>(node.decl.get())) {
        // if we are declaring a constant, mov it directly.
        auto reg = findFreeReg();

        num->accept(*this);
        program[pc] = 2;
        program[pc + 1] = reg;
        program[pc + 2] = tmp[0];
        pc += 3;

        vars[reg] = node.name;
    } else if (auto binop = dynamic_cast<BinaryOperatorNode *>(node.decl.get())) {
        // if we are declaring a binop, let it handle itself and claim its
        // register under our name.
        binop->accept(*this);
        vars[tmp[0]] = node.name;
        std::cout << std::endl;
    } else if (auto var = dynamic_cast<VariableRefNode *>(node.decl.get())) {
        // if we are declaring the same as another variable, copy it.
        auto from = findVar(var->name);
        auto to = findFreeReg();

        program[pc] = 3;
        program[pc + 1] = to;
        program[pc + 2] = from;
        pc += 3;

        vars[to] = node.name;
    }
    return nullptr;
}

std::unique_ptr<IRNode> LoweringVisitor::visit(BinaryOperatorNode &node) {
    if (auto var = dynamic_cast<VariableRefNode *>(node.lhs.get())) {
        if (auto num = dynamic_cast<NumberNode *>(node.rhs.get())) {
            // find the variable's register, then run reg <op> const
            auto reg = findVar(var->name);
            auto out = findFreeReg();
            program[pc] = makeOperator(node.op) + 5;
            program[pc + 1] = reg;
            program[pc + 2] = num->value;
            program[pc + 3] = out;
            pc += 4;
            tmp[0] = out;
        } else if (auto varR = dynamic_cast<VariableRefNode *>(node.rhs.get())) {
            auto reg1 = findVar(var->name);
            auto reg2 = findVar(varR->name);
            auto out = findFreeReg();
            program[pc] = makeOperator(node.op);
            program[pc + 1] = reg1;
            program[pc + 2] = reg2;
            program[pc + 3] = out;
            pc += 4;
            tmp[0] = out;
        }
    } else if (auto num = dynamic_cast<NumberNode *>(node.lhs.get())) {
        if (auto var = dynamic_cast<VariableRefNode *>(node.rhs.get())) {
            auto reg = findVar(var->name);
            auto out = findFreeReg();
            auto op = makeOperator(node.op) + 5;
            if (node.op != Op::Add && node.op != Op::Mul) {
                // find the right operation case.
                op += 3; // div, pow
                if (node.op == Op::Sub)
                    op += 1; // sub
                program[pc] = op;
                program[pc + 1] = num->value;
                program[pc + 2] = reg;
            } else {
                // reverse order and use reg +/* const.
                program[pc] = op;
                program[pc + 1] = reg;
                program[pc + 2] = num->value;
            }
            program[pc + 3] = out;
            pc += 4;
            tmp[0] = out;
        }
    }
    return nullptr;
}

std::unique_ptr<IRNode> LoweringVisitor::visit(SequenceNode &node) {
    std::for_each(node.body.begin(), node.body.end(),
                  [this](std::unique_ptr<IRNode> &elem) { elem->accept(*this); });
    return nullptr;
}

std::unique_ptr<IRNode> LoweringVisitor::visit(BlockNode &node) {
    // record the position, then call the rest.
    blocks[node.label] = pc;
    node.body->accept(*this);
    return nullptr;
}

std::unique_ptr<IRNode> LoweringVisitor::visit(BlockCallNode &node) {
    // jump to position
    program[pc] = 26;
    program[pc + 1] = blocks[node.label] + 8; // header
    pc += 2;
    return nullptr;
}

std::unique_ptr<IRNode> LoweringVisitor::visit(ParamNode &node) {
    node.param->accept(*this);
    program[pc] = 10;
    program[pc + 1] = tmp[0];
    pc += 2;
    return nullptr;
}
} // namespace ir
