#pragma once

#include <fstream>

// fwd-decls of ast.hpp nodes.
class NumberNode;
class BinaryOperatorNode;

class Visitor {
public:
    virtual void visit(NumberNode& node) = 0;
    virtual void visit(BinaryOperatorNode& node) = 0;
    virtual ~Visitor() = default;
};

class PrintVisitor : public Visitor {
public:
    void visit(NumberNode& node) override;
    void visit(BinaryOperatorNode& node) override;
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
    void render();
};
