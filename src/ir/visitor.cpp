#include <algorithm>
#include <iostream>

#include "ir/ir.hpp"
#include "ir/visitor.hpp"

// TODO: program[++pc] faster?

namespace ir {
void PrintVisitor::visit(const NumberNode &node) { std::cout << node.value; }

void PrintVisitor::visit(const VariableRefNode &node) { std::cout << node.name; }

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
    std::for_each(node.body.begin(), node.body.end(), [this](const std::unique_ptr<IRNode> &stmt) {
        stmt->accept(*this);
        std::cout << std::endl;
    });
}

void PrintVisitor::visit(const BlockNode &node) {
    std::cout << "LABEL " << node.label << std::endl;

    prefix += "    ";
    if (node.body)
        node.body->accept(*this);
    prefix.erase(prefix.size() - 4);
}

void PrintVisitor::visit(const BlockCallNode &node) {
    std::cout << prefix << "CALL " << node.label << " TO " << node.outName;
}

void PrintVisitor::visit(const ParamNode &node) {
    std::cout << prefix << "PARAM ";
    node.param->accept(*this);
}

void PrintVisitor::visit(const PrintNode &node) { std::cout << prefix << "PRINT"; }

void StubVisitor::visit(const NumberNode &node) {}

void StubVisitor::visit(const VariableRefNode &node) {}

void StubVisitor::visit(const VariableDeclNode &node) { node.decl->accept(*this); }

void StubVisitor::visit(const BinaryOperatorNode &node) {
    node.lhs->accept(*this);
    node.rhs->accept(*this);
}

void StubVisitor::visit(const SequenceNode &node) {
    std::for_each(node.body.begin(), node.body.end(),
                  [this](const std::unique_ptr<IRNode> &stmt) { stmt->accept(*this); });
}

void StubVisitor::visit(const BlockNode &node) {
    // write a blank stub
    stubs[pc] = 26;
    stubs[pc + 1] = 0;
    blocks[node.label] = pc;
    pc += 2;
}

void StubVisitor::visit(const BlockCallNode &node) {}

void StubVisitor::visit(const ParamNode &node) {}

void StubVisitor::visit(const PrintNode &node) {}

void LoweringVisitor::visit(const NumberNode &node) { tmp[0] = node.value; }

void LoweringVisitor::visit(const VariableRefNode &node) { tmp[0] = findVar(node.name); }

void LoweringVisitor::visit(const VariableDeclNode &node) {
    if (auto num = dynamic_cast<NumberNode *>(node.decl.get())) {
        // if we are declaring a constant, move it directly.
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
        var->accept(*this);
        auto to = findFreeReg();

        program[pc] = 3;
        program[pc + 1] = to;
        program[pc + 2] = tmp[0];
        pc += 3;

        vars[to] = node.name;
    }
}

void LoweringVisitor::visit(const BinaryOperatorNode &node) {
    if (auto var = dynamic_cast<VariableRefNode *>(node.lhs.get())) {
        if (auto num = dynamic_cast<NumberNode *>(node.rhs.get())) {
            // find the variable's register, then run reg <op> const
            var->accept(*this);
            auto out = findFreeReg();
            program[pc] = makeOperator(node.op, true, false);
            program[pc + 1] = tmp[0];
            program[pc + 2] = num->value;
            program[pc + 3] = out;
            pc += 4;
            tmp[0] = out;
        } else if (auto varR = dynamic_cast<VariableRefNode *>(node.rhs.get())) {
            var->accept(*this);
            auto reg1 = tmp[0];
            varR->accept(*this);
            auto reg2 = tmp[0];

            auto out = findFreeReg();
            program[pc] = makeOperator(node.op, true, true);
            program[pc + 1] = reg1;
            program[pc + 2] = reg2;
            program[pc + 3] = out;
            pc += 4;
            tmp[0] = out;
        }
    } else if (auto num = dynamic_cast<NumberNode *>(node.lhs.get())) {
        if (auto var = dynamic_cast<VariableRefNode *>(node.rhs.get())) {
            var->accept(*this);
            auto out = findFreeReg();
            program[pc] = makeOperator(node.op, false, true);
            program[pc + 1] = num->value;
            program[pc + 2] = tmp[0];
            program[pc + 3] = out;
            pc += 4;
            tmp[0] = out;
        }
    }
}

void LoweringVisitor::visit(const SequenceNode &node) {
    std::for_each(node.body.begin(), node.body.end(),
                  [this](const std::unique_ptr<IRNode> &elem) { elem->accept(*this); });
}

void LoweringVisitor::visit(const BlockNode &node) {
    std::cout << "lowering a block node" << std::endl;
    // first rename parameters.
    for (int i = 0; i < node.params.size(); ++i) {
        // TODO: horrible
        auto x = std::distance(
            std::begin(vars), std::find(std::begin(vars), std::end(vars), "p" + std::to_string(i)));
        std::cout << "found " << x << std::endl;
        for (int m = 0; m < 64; ++m) {
            std::cout << "  " << vars[m] << std::endl;
        }
        vars[findFreeReg()] = vars[x];
    }
    std::cout << "all params renamed" << std::endl;
    // check the stubs.
    if (blocks.contains(node.label)) {
        // find free section of function defs and run.
        auto pcOld = pc;
        pc = pcDef;
        program[pc++] = 0;

        // modify stub to point to def after buf
        program[blocks[node.label] + 32760 + 1] = pc + 8; // program + 1 = pc + header

        // if the program reaches this block (without being called), stop.
        node.body->accept(*this);

        // update pcDef, reset pc.
        pcDef += pc - pcOld; // length of the block
        pc = pcOld;
    } else {
        // should not reach.
        throw std::runtime_error("internal compiler error: stub not found");
    }
}

void LoweringVisitor::visit(const BlockCallNode &node) {
    // jump to position.
    program[pc] = 26;
    program[pc + 1] = blocks[node.label] + 32768; // memory header + program + buffer
    pc += 2;
    // TODO: outname
}

void LoweringVisitor::visit(const ParamNode &node) {
    if (auto num = dynamic_cast<NumberNode *>(node.param.get())) {
        auto reg = findFreeReg();
        num->accept(*this);
        program[pc] = 2;
        program[pc + 1] = reg;
        program[pc + 2] = tmp[0];
        pc += 3;

        vars[reg] = node.name;
    } else if (auto var = dynamic_cast<VariableRefNode *>(node.param.get())) {
        var->accept(*this);
        if (tmp[0] != 65535) {
            auto reg = findFreeReg();
            program[pc] = 2;
            program[pc + 1] = reg;
            program[pc + 2] = tmp[0];
            pc += 3;

            vars[reg] = node.name;
        } else {
            throw std::runtime_error("param uses unknown variable");
        }
    }
}

void LoweringVisitor::visit(const PrintNode &node) {
    program[pc] = 36;
    ++pc;
}
} // namespace ir
