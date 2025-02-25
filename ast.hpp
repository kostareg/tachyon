#pragma once

#include <memory>
#include <string>

#include "visitor.hpp"

/// Base class for AST Nodes.
class ASTNode {
public:
    virtual void accept(Visitor& visitor) = 0;
    virtual ~ASTNode() = default;
};

// Number Node (Leaf)
class NumberNode : public ASTNode {
public:
    int value;
    explicit NumberNode(int val) : value(val) {}

    void accept(Visitor& visitor) {
        visitor.visit(*this);
    }
};

enum class Op { Add, Sub, Mul, Div, Pow };

inline std::string op_to_string(Op op) {
    switch (op) {
        case Op::Add: return "+";
        case Op::Sub: return "-";
        case Op::Mul: return "*";
        case Op::Div: return "/";
        case Op::Pow: return "^";
        default: return "unknown";
    }
}

// Operator Node (Internal Node)
class BinaryOperatorNode : public ASTNode {
public:
    Op op;
    std::unique_ptr<ASTNode> left;
    std::unique_ptr<ASTNode> right;

    BinaryOperatorNode(Op op, std::unique_ptr<ASTNode> left, std::unique_ptr<ASTNode> right)
        : op(op), left(std::move(left)), right(std::move(right)) {}

    void accept(Visitor& visitor) {
        visitor.visit(*this);
    }
};

// Function to evaluate the AST
int evaluateAST(const ASTNode* node);

