#pragma once

#include <fstream>

#include "ast/visitors/visitor.hpp"

namespace ast {
/**
 * @brief create a graphviz flow chart.
 */
class TreeVisitor : public Visitor {
    std::ofstream file;
    std::string root;

  public:
    TreeVisitor() : file("./build/ast.dot") {
        file << "digraph AST{\nnode [shape=box];\n";
    }
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
} // namespace ast
