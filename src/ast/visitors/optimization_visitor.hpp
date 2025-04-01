#pragma once

#include <memory>
#include <optional>
#include <vector>

#include "ast/ast.hpp"
#include "ast/visitors/visitor.hpp"
#include "error.hpp"

namespace ast {
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
    std::optional<Error> error;
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
} // namespace ast
