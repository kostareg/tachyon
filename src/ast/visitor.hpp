#pragma once

#include <fstream>
#include <memory>
#include <vector>

#include "ir/ir.hpp"

namespace ast {
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

/**
 * @brief Visitor pattern for abstract syntax tree.
 * @sa ast::ASTNode
 * @sa ast::ASTNode::accept()
 *
 * Derive this class in order to use `ASTNode::accept(Visitor &visitor)`.
 * Each `Visitor::visit` implementation receives immutable access to the node.
 */
class Visitor {
  public:
    virtual void visit(const NumberNode &node) = 0;
    virtual void visit(const BinaryOperatorNode &node) = 0;
    virtual void visit(const VariableDeclNode &node) = 0;
    virtual void visit(const VariableRefNode &node) = 0;
    virtual void visit(const FunctionDefNode &node) = 0;
    virtual void visit(const FunctionCallNode &node) = 0;
    virtual void visit(const SequenceNode &node) = 0;
    virtual ~Visitor() = default;
};

/**
 * @brief Formatted print.
 */
class PrintVisitor : public Visitor {
  public:
    void visit(const NumberNode &node) override;
    void visit(const BinaryOperatorNode &node) override;
    void visit(const VariableDeclNode &node) override;
    void visit(const VariableRefNode &node) override;
    void visit(const FunctionDefNode &node) override;
    void visit(const FunctionCallNode &node) override;
    void visit(const SequenceNode &node) override;
};

/**
 * @brief create a graphviz flow chart.
 */
class TreeVisitor : public Visitor {
    std::ofstream file;
    std::string root;

  public:
    TreeVisitor() : file("./build/ast.dot") { file << "digraph AST{\nnode [shape=box];\n"; }
    void visit(const NumberNode &node) override;
    void visit(const BinaryOperatorNode &node) override;
    void visit(const VariableDeclNode &node) override;
    void visit(const VariableRefNode &node) override;
    void visit(const FunctionDefNode &node) override;
    void visit(const FunctionCallNode &node) override;
    void visit(const SequenceNode &node) override;

    /**
     * @brief Show the flow chart.
     */
    void render();
};

/**
 * @brief First pass optimizations.
 *
 * Applies the following optimizations:
 *   1. Constant folding
 *   2. TODO: Dead code optimization
 *     i. TODO: Logically unreachable statements

 * This pass is NOT responsible for:
 *   1. Constant propagation (JIT)
 *   2. Dead code optimization (2nd pass)
 *     i. Statically unused variables

 * This pass also collects information on which static variables are used.
 * This data will be used by the second pass for dead code optimization of
 * unused variables.
 *
 * TODO: also store list of functions to double check.
 */
class OptimizationVisitor1 : public Visitor {
  public:
    std::unique_ptr<ASTNode> optimizedNode;
    std::vector<std::string> varsReferenced;
    bool root = true;
    void visit(const NumberNode &node) override;
    void visit(const BinaryOperatorNode &node) override;
    void visit(const VariableDeclNode &node) override;
    void visit(const VariableRefNode &node) override;
    void visit(const FunctionDefNode &node) override;
    void visit(const FunctionCallNode &node) override;
    void visit(const SequenceNode &node) override;

    int computeBinaryOp(::Op op, int left, int right);
};

/*
 * @brief Second pass optimizations.
 *
 * Applies the following optimizations:
 *   1. Dead code optimization
 *     i. Statically unused variables (a)
 *
 *  (a) Only does one pass of statically unused variable. The JIT will really
 *      removing anything important by design.
 */
class OptimizationVisitor2 : public Visitor {
  public:
    std::unique_ptr<ASTNode> optimizedNode;
    std::vector<std::string> varsReferenced;

    OptimizationVisitor2(std::vector<std::string> varsReferenced)
        : varsReferenced(varsReferenced) {}

    void visit(const NumberNode &node) override;
    void visit(const BinaryOperatorNode &node) override;
    void visit(const VariableDeclNode &node) override;
    void visit(const VariableRefNode &node) override;
    void visit(const FunctionDefNode &node) override;
    void visit(const FunctionCallNode &node) override;
    void visit(const SequenceNode &node) override;
};

/**
 * @brief Lower to intermediate representation.
 * @sa ir::IRNode
 */
class LoweringVisitor : public Visitor {
  public:
    std::vector<std::unique_ptr<ir::IRNode>> loweredNodes;
    std::unique_ptr<ir::IRNode> tmp;
    int tmpInt = 0;

    void visit(const NumberNode &node) override;
    void visit(const BinaryOperatorNode &node) override;
    void visit(const VariableDeclNode &node) override;
    void visit(const VariableRefNode &node) override;
    void visit(const FunctionDefNode &node) override;
    void visit(const FunctionCallNode &node) override;
    void visit(const SequenceNode &node) override;

    std::string tmpVar() { return "t" + std::to_string(++tmpInt); }
};
} // namespace ast
