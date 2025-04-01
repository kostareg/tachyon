#include <spdlog/spdlog.h>

#include "ast/ast.hpp"
#include "optimization_visitor.hpp"

namespace ast {
void OptimizationVisitor1::visit(const NumberNode &node) {
    if (root)
        return;
    optimizedNode = std::make_unique<NumberNode>(node.value);
}

void OptimizationVisitor1::visit(const BinaryOperatorNode &node) {
    if (root)
        return;
    root = false;
    node.lhs->accept(*this);
    if (error)
        return;
    auto left = std::move(optimizedNode);
    node.rhs->accept(*this);
    if (error)
        return;
    auto right = std::move(optimizedNode);

    // apply constant folding if both hands are NumberNodes.
    if (auto leftNum = dynamic_cast<NumberNode *>(left.get())) {
        if (auto rightNum = dynamic_cast<NumberNode *>(right.get())) {
            double result =
                computeBinaryOp(node.op, leftNum->value, rightNum->value);
            optimizedNode = std::make_unique<NumberNode>(result);
            return;
        }
    }

    // could not optimize, return optimized children.
    optimizedNode = std::make_unique<BinaryOperatorNode>(
        node.op, std::move(left), std::move(right));
}

void OptimizationVisitor1::visit(const VariableDeclNode &node) {
    if (error)
        return;
    root = false;
    node.decl->accept(*this);
    if (error)
        return;
    optimizedNode =
        std::make_unique<VariableDeclNode>(node.name, std::move(optimizedNode));
}

void OptimizationVisitor1::visit(const VariableRefNode &node) {
    varsReferenced.push_back(node.name);
    optimizedNode = std::make_unique<VariableRefNode>(node.name);
}

void OptimizationVisitor1::visit(const FunctionDefNode &node) {
    root = true;
    node.body->accept(*this);
    if (error)
        return;
    if (optimizedNode)
        optimizedNode = std::make_unique<FunctionDefNode>(
            node.name, std::move(node.args), std::move(optimizedNode));
}

void OptimizationVisitor1::visit(const FunctionCallNode &node) {
    root = false;
    std::vector<std::unique_ptr<ASTNode>> args;
    std::for_each(node.args.begin(), node.args.end(),
                  [this, &args](const std::unique_ptr<ASTNode> &arg) {
                      arg->accept(*this);
                      if (error)
                          return;
                      args.push_back(std::move(optimizedNode));
                  });
    optimizedNode =
        std::make_unique<FunctionCallNode>(node.name, std::move(args));
}

void OptimizationVisitor1::visit(const SequenceNode &node) {
    std::vector<std::unique_ptr<ASTNode>> stmts;
    std::for_each(node.stmts.begin(), node.stmts.end(),
                  [this, &stmts](const std::unique_ptr<ASTNode> &stmt) {
                      spdlog::trace("opt1 sequence node");
                      root = true;
                      stmt->accept(*this);
                      if (error)
                          return;
                      if (optimizedNode)
                          stmts.push_back(std::move(optimizedNode));
                  });
    optimizedNode = std::make_unique<SequenceNode>(std::move(stmts));
}

int OptimizationVisitor1::computeBinaryOp(::Op op, int left, int right) {
    switch (op) {
    case ::Op::Add:
        return left + right;
    case ::Op::Sub:
        return left - right;
    case ::Op::Mul:
        return left * right;
    case ::Op::Div:
        return left / right;
    case ::Op::Pow:
        return pow(left, right);
    default:
        throw std::runtime_error("unknown operator");
    }
}

void OptimizationVisitor2::visit(const NumberNode &node) {
    optimizedNode = std::make_unique<NumberNode>(node.value);
};

void OptimizationVisitor2::visit(const BinaryOperatorNode &node) {
    node.lhs->accept(*this);
    auto left = std::move(optimizedNode);
    node.rhs->accept(*this);
    auto right = std::move(optimizedNode);
    optimizedNode = std::make_unique<BinaryOperatorNode>(
        node.op, std::move(left), std::move(right));
};

void OptimizationVisitor2::visit(const VariableDeclNode &node) {
    // if unused, skip.
    if (find(varsReferenced.begin(), varsReferenced.end(), node.name) ==
        varsReferenced.end()) {
        return;
    }

    node.decl->accept(*this);
    optimizedNode =
        std::make_unique<VariableDeclNode>(node.name, std::move(optimizedNode));
};

void OptimizationVisitor2::visit(const VariableRefNode &node) {
    optimizedNode = std::make_unique<VariableRefNode>(node.name);
};

void OptimizationVisitor2::visit(const FunctionDefNode &node) {
    node.body->accept(*this);
    optimizedNode = std::make_unique<FunctionDefNode>(
        node.name, std::move(node.args), std::move(optimizedNode));
};

void OptimizationVisitor2::visit(const FunctionCallNode &node) {
    std::vector<std::unique_ptr<ASTNode>> args;
    std::for_each(node.args.begin(), node.args.end(),
                  [this, &args](const std::unique_ptr<ASTNode> &arg) {
                      arg->accept(*this);
                      args.push_back(std::move(optimizedNode));
                  });
    optimizedNode =
        std::make_unique<FunctionCallNode>(node.name, std::move(args));
};

void OptimizationVisitor2::visit(const SequenceNode &node) {
    std::vector<std::unique_ptr<ASTNode>> stmts;
    std::for_each(node.stmts.begin(), node.stmts.end(),
                  [this, &stmts](const std::unique_ptr<ASTNode> &stmt) {
                      stmt->accept(*this);
                      if (optimizedNode)
                          stmts.push_back(std::move(optimizedNode));
                  });
    optimizedNode = std::make_unique<SequenceNode>(std::move(stmts));
};
} // namespace ast
