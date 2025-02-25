#pragma once

#include <fstream>
#include <memory>
#include <unordered_map>

// fwd-decls of ast.hpp nodes.
class ASTNode;
class NumberNode;
class BinaryOperatorNode;
class VariableDeclNode;
class VariableRefNode;
class FunctionDefNode;
class FunctionCallNode;
class SequenceNode;
enum class Op;

class Visitor {
public:
    virtual void visit(NumberNode& node) = 0;
    virtual void visit(BinaryOperatorNode& node) = 0;
    virtual void visit(VariableDeclNode& node) = 0;
    virtual void visit(VariableRefNode& node) = 0;
    virtual void visit(FunctionDefNode& node) = 0;
    virtual void visit(FunctionCallNode& node) = 0;
    virtual void visit(SequenceNode& node) = 0;
    virtual ~Visitor() = default;
};

class PrintVisitor : public Visitor {
public:
    void visit(NumberNode& node) override;
    void visit(BinaryOperatorNode& node) override;
    void visit(VariableDeclNode& node) override;
    void visit(VariableRefNode& node) override;
    void visit(FunctionDefNode& node) override;
    void visit(FunctionCallNode& node) override;
    void visit(SequenceNode& node) override;
};

class TreeVisitor : public Visitor {
    std::ofstream file;
    std::string root = "Root";
public:
    TreeVisitor() : file("./build/ast.dot") {
        file << "digraph AST{\n";
    }
    void visit(NumberNode& node) override;
    void visit(BinaryOperatorNode& node) override;
    void visit(VariableDeclNode& node) override;
    void visit(VariableRefNode& node) override;
    void visit(FunctionDefNode& node) override;
    void visit(FunctionCallNode& node) override;
    void visit(SequenceNode& node) override;
    void render();
};

class OptimizationVisitor : public Visitor {
public:
    std::unique_ptr<ASTNode> optimizedNode;
    std::unordered_map<std::string, std::unique_ptr<ASTNode>> constants;
    void visit(NumberNode& node) override;
    void visit(BinaryOperatorNode& node) override;
    void visit(VariableDeclNode& node) override;
    void visit(VariableRefNode& node) override;
    void visit(FunctionDefNode& node) override;
    void visit(FunctionCallNode& node) override;
    void visit(SequenceNode& node) override;
    int computeBinaryOp(Op op, int left, int right);
};

