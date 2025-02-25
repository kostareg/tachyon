#pragma once

#include <memory>
#include <string>
#include <vector>

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

class VariableDeclNode : public ASTNode {
public:
    std::string name;
    std::unique_ptr<ASTNode> decl;

    VariableDeclNode(std::string name, std::unique_ptr<ASTNode> decl) : name(std::move(name)), decl(std::move(decl)) {}

    void accept(Visitor& visitor) {
        visitor.visit(*this);
    }
};

class VariableRefNode : public ASTNode {
public:
    std::string name;

    VariableRefNode(std::string name) : name(std::move(name)) {}

    void accept(Visitor& visitor) {
        visitor.visit(*this);
    }
};

class FunctionDefNode : public ASTNode {
public:
    std::string name;
    std::vector<std::string> args;
    std::unique_ptr<ASTNode> body;

    FunctionDefNode(std::string name, std::vector<std::string> args, std::unique_ptr<ASTNode> body) : name(std::move(name)), args(std::move(args)), body(std::move(body)) {}

    void accept(Visitor& visitor) {
        visitor.visit(*this);
    }
};

class FunctionCallNode : public ASTNode {
public:
    std::string name;
    std::vector<std::unique_ptr<ASTNode>> args;

    FunctionCallNode(std::string name, std::vector<std::unique_ptr<ASTNode>>&& args) : name(std::move(name)), args(std::move(args)) {}

    void accept(Visitor& visitor) {
        visitor.visit(*this);
    }
};

class SequenceNode : public ASTNode {
public:
    std::vector<std::unique_ptr<ASTNode>> stmts;

    SequenceNode(std::vector<std::unique_ptr<ASTNode>>&& stmts) : stmts(std::move(stmts)) {}

    void accept(Visitor& visitor) {
        visitor.visit(*this);
    }
};

// Function to evaluate the AST
int evaluateAST(const ASTNode* node);
