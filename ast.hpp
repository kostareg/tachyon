#pragma once

#include <memory>
#include <string>
#include <vector>

#include "visitor.hpp"
#include "op.hpp"

/// Base class for AST Nodes.
class ASTNode {
public:
    virtual void accept(Visitor& visitor) = 0;
    virtual std::unique_ptr<ASTNode> clone() const = 0; // TODO: rm all?
    virtual ~ASTNode() = default;
};

// Number Node (Leaf)
class NumberNode : public ASTNode {
public:
    int value;
    explicit NumberNode(int val) : value(val) {} // TODO: explicit?

    void accept(Visitor& visitor) override {
        visitor.visit(*this);
    }

    std::unique_ptr<ASTNode> clone() const override {
        return std::make_unique<NumberNode>(*this);
    }
};

// Operator Node (Internal Node)
class BinaryOperatorNode : public ASTNode {
public:
    Op op;
    std::unique_ptr<ASTNode> left;
    std::unique_ptr<ASTNode> right;

    BinaryOperatorNode(Op op, std::unique_ptr<ASTNode> left, std::unique_ptr<ASTNode> right)
        : op(op), left(std::move(left)), right(std::move(right)) {}

    BinaryOperatorNode(const BinaryOperatorNode& other)
        : op(other.op),
          left(other.left ? other.left->clone() : nullptr),
          right(other.right ? other.right->clone() : nullptr) {}

    void accept(Visitor& visitor) override {
        visitor.visit(*this);
    }

    std::unique_ptr<ASTNode> clone() const override {
        return std::make_unique<BinaryOperatorNode>(*this);
    }
};

class VariableDeclNode : public ASTNode {
public:
    std::string name;
    std::unique_ptr<ASTNode> decl;

    VariableDeclNode(std::string name, std::unique_ptr<ASTNode> decl) : name(std::move(name)), decl(std::move(decl)) {}

    VariableDeclNode(const VariableDeclNode& other)
        : name(other.name),
          decl(other.decl ? other.decl->clone() : nullptr) {}

    void accept(Visitor& visitor) override {
        visitor.visit(*this);
    }

    std::unique_ptr<ASTNode> clone() const override {
        return std::make_unique<VariableDeclNode>(*this);
    }
};

class VariableRefNode : public ASTNode {
public:
    std::string name;

    VariableRefNode(std::string name) : name(std::move(name)) {}

    void accept(Visitor& visitor) override {
        visitor.visit(*this);
    }

    std::unique_ptr<ASTNode> clone() const override {
        return std::make_unique<VariableRefNode>(*this);
    }
};

class FunctionDefNode : public ASTNode {
public:
    std::string name;
    std::vector<std::string> args;
    std::unique_ptr<ASTNode> body; // TODO: seq?

    FunctionDefNode(std::string name, std::vector<std::string> args, std::unique_ptr<ASTNode> body) : name(std::move(name)), args(std::move(args)), body(std::move(body)) {}

    FunctionDefNode(const FunctionDefNode& other)
        : name(other.name),
          args(other.args),
          body(other.body ? other.body->clone() : nullptr) {}

    void accept(Visitor& visitor) override {
        visitor.visit(*this);
    }

    std::unique_ptr<ASTNode> clone() const override {
        return std::make_unique<FunctionDefNode>(*this);
    }
};

class FunctionCallNode : public ASTNode {
public:
    std::string name;
    std::vector<std::unique_ptr<ASTNode>> args;

    FunctionCallNode(std::string name, std::vector<std::unique_ptr<ASTNode>>&& args) : name(std::move(name)), args(std::move(args)) {}

    FunctionCallNode(const FunctionCallNode& other)
        : name(other.name) {
        for (const auto& arg : other.args) {
            args.push_back(arg ? arg->clone() : nullptr);
        }
    }

    void accept(Visitor& visitor) override {
        visitor.visit(*this);
    }

    std::unique_ptr<ASTNode> clone() const override {
        return std::make_unique<FunctionCallNode>(*this);
    }
};

class SequenceNode : public ASTNode {
public:
    std::vector<std::unique_ptr<ASTNode>> stmts;

    SequenceNode(std::vector<std::unique_ptr<ASTNode>>&& stmts) : stmts(std::move(stmts)) {}

    SequenceNode(const SequenceNode& other) {
        for (const auto& stmt : other.stmts) {
            stmts.push_back(stmt ? stmt->clone() : nullptr);
        }
    }

    void accept(Visitor& visitor) override {
        visitor.visit(*this);
    }

    std::unique_ptr<ASTNode> clone() const override {
        return std::make_unique<SequenceNode>(*this);
    }
};

