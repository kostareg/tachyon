#pragma once

#include <fstream>
#include <memory>
#include <vector>

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
    std::string root;
public:
    TreeVisitor() : file("./build/ast.dot") {
        file << "digraph AST{\nnode [shape=box];\n";
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

/// The first pass of AST optimization applies the following optimizations:
///   1. Constant folding
///   2. Dead code optimization
///     i. TODO: logically unreachable statements
/// 
/// This pass is NOT responsible for:
///   1. Constant propagation (JIT)
///   2. Dead code optimization (2nd pass)
///     i. Statically unused variables
///
/// This pass also collects information on which static variables are used.
/// This data will be used by the second pass for dead code optimization of
/// unused variables.
class OptimizationVisitor1 : public Visitor {
public:
    std::unique_ptr<ASTNode> optimizedNode;
    std::vector<std::string> varsReferenced;
    bool root = true;
    void visit(NumberNode& node) override;
    void visit(BinaryOperatorNode& node) override;
    void visit(VariableDeclNode& node) override;
    void visit(VariableRefNode& node) override;
    void visit(FunctionDefNode& node) override;
    void visit(FunctionCallNode& node) override;
    void visit(SequenceNode& node) override;
    int computeBinaryOp(Op op, int left, int right);
};

/// Second pass.
///   1. Dead code optimization
///     i. Statically unused variables
class OptimizationVisitor2 : public Visitor {
public:
    std::unique_ptr<ASTNode> optimizedNode;
    std::vector<std::string> varsReferenced;

    OptimizationVisitor2(std::vector<std::string> varsReferenced) : varsReferenced(varsReferenced) {}

    void visit(NumberNode& node) override;
    void visit(BinaryOperatorNode& node) override;
    void visit(VariableDeclNode& node) override;
    void visit(VariableRefNode& node) override;
    void visit(FunctionDefNode& node) override;
    void visit(FunctionCallNode& node) override;
    void visit(SequenceNode& node) override;
};

