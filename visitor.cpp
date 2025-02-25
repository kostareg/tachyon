#include "visitor.hpp"
#include "ast.hpp"

#include <iostream>
#include <cstdlib>

void PrintVisitor::visit(NumberNode& node) {
    std::cout << node.value;
}

void PrintVisitor::visit(BinaryOperatorNode& node) {
    std::cout << "(";
    node.left->accept(*this);
    std::cout << " " << op_to_string(node.op) << " ";
    node.right->accept(*this);
    std::cout << ")";
}

void TreeVisitor::visit(NumberNode& node) {
    auto ident = std::to_string(reinterpret_cast<uintptr_t>(&node));
    file << ident << " [label=\"" << node.value << "\"];\n";
    file << root << " -> " << ident << ";\n";
}

void TreeVisitor::visit(BinaryOperatorNode& node) {
    auto rootO = root;
    auto ident = std::to_string(reinterpret_cast<uintptr_t>(&node));
    file << ident << " [label=\"" << op_to_string(node.op) << "\"];\n";
    file << root << " -> " << ident << ";\n";

    // change the root for lhs and rhs, then set it back to original.
    root = ident;

    node.left->accept(*this);
    node.right->accept(*this);

    root = rootO;
}

void TreeVisitor::render() {
    file << "}\n";
    file.flush();
    file.close();
    int result = std::system("/nix/store/lvx8m4mh18aiqlqp5hdjpg3b34a07p35-graphviz-12.2.0/bin/dot -Tpng ./build/ast.dot -o ./build/ast.png");

    if (result != 0) {
        std::cerr << "Error: Graphviz failed to generate ./build/ast.png" << std::endl;
    }

    std::cout << std::endl << "Viewing with feh. Press ESC to exit." << std::endl;
    std::cout.flush();
    std::system("feh ./build/ast.png");
}
