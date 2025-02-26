#include "ir.hpp"

#include <algorithm>
#include <iostream>

using namespace ir;

std::unique_ptr<IRNode> PrintVisitor::visit(NumberNode& node) {
    std::cout << node.value;
    return nullptr;
}

std::unique_ptr<IRNode> PrintVisitor::visit(VariableRefNode& node) {
    std::cout << node.name;
    return nullptr;
}

std::unique_ptr<IRNode> PrintVisitor::visit(VariableDeclNode& node) {
    std::cout << prefix << node.name << " = ";
    node.decl->accept(*this);
    return nullptr;
}

std::unique_ptr<IRNode> PrintVisitor::visit(BinaryOperatorNode& node) {
    auto prefixO = prefix;
    prefix = "";
    node.lhs->accept(*this);
    std::cout << " " << op_to_string(node.op) << " ";
    node.rhs->accept(*this);
    prefix = prefixO;
    return nullptr;
}

std::unique_ptr<IRNode> PrintVisitor::visit(SequenceNode& node) {
    std::for_each(node.body.begin(), node.body.end(), [this](std::unique_ptr<IRNode>& stmt) {
        stmt->accept(*this);
        std::cout << std::endl;
    });
    return nullptr;
}

std::unique_ptr<IRNode> PrintVisitor::visit(BlockNode& node) {
    std::cout << "LABEL " << node.label << std::endl;

    prefix += "    ";
    if (node.body) node.body->accept(*this);
    prefix.erase(prefix.size() - 4);

    return nullptr;
}

std::unique_ptr<IRNode> PrintVisitor::visit(BlockCallNode& node) {
    std::cout << prefix << "CALL " << node.label << " TO " << node.outName;
    return nullptr;
}

std::unique_ptr<IRNode> PrintVisitor::visit(ParamNode& node) {
    std::cout << prefix << "PARAM ";
    node.param->accept(*this);
    return nullptr;
}

