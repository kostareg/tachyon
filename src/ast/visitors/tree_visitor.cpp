#include <iostream>

#include "ast/ast.hpp"
#include "tree_visitor.hpp"

namespace ast {
void TreeVisitor::visit(const NumberNode &node) {
    auto ident = std::to_string(reinterpret_cast<uintptr_t>(&node));
    file << ident << " [label=\"" << node.value << "\"];\n";
    if (!root.empty())
        file << root << " -> " << ident << ";\n";
}

void TreeVisitor::visit(const BinaryOperatorNode &node) {
    auto rootO = root;
    auto ident = std::to_string(reinterpret_cast<uintptr_t>(&node));
    file << ident << " [label=\"" << op_to_str(node.op) << "\"];\n";
    if (!root.empty())
        file << root << " -> " << ident << ";\n";

    // change the root for lhs and rhs, then set it back to original.
    root = ident;

    node.lhs->accept(*this);
    node.rhs->accept(*this);

    root = rootO;
}

void TreeVisitor::visit(const VariableDeclNode &node) {
    auto rootO = root;
    auto ident = std::to_string(reinterpret_cast<uintptr_t>(&node));
    file << ident << " [label=\"" << node.name << "\"];\n";
    if (!root.empty())
        file << root << " -> " << ident << ";\n";

    root = ident;

    node.decl->accept(*this);

    root = rootO;
}

void TreeVisitor::visit(const VariableRefNode &node) {
    auto ident = std::to_string(reinterpret_cast<uintptr_t>(&node));
    file << ident << " [label=\"" << node.name << "\"];\n";
    if (!root.empty())
        file << root << " -> " << ident << ";\n";
}

void TreeVisitor::visit(const FunctionDefNode &node) {
    auto rootO = root;
    auto ident = std::to_string(reinterpret_cast<uintptr_t>(&node));
    file << ident << " [label=\"" << node.name << "(";
    std::for_each(node.args.begin(), node.args.end(),
                  [this](const std::string &arg) { file << arg << ","; });
    file << ")\"];\n";
    if (!root.empty())
        file << root << " -> " << ident << ";\n";

    root = ident;

    node.body->accept(*this);

    root = rootO;
}

void TreeVisitor::visit(const FunctionCallNode &node) {
    auto rootO = root;
    auto ident = std::to_string(reinterpret_cast<uintptr_t>(&node));
    file << ident << " [label=\"" << node.name << "()\"];\n";
    if (!root.empty())
        file << root << " -> " << ident << ";\n";

    root = ident;

    std::for_each(node.args.begin(), node.args.end(),
                  [this](const std::unique_ptr<ASTNode> &arg) {
                      arg->accept(*this);
                      std::cout << ",";
                  });

    root = rootO;
}

void TreeVisitor::visit(const SequenceNode &node) {
    auto ident = std::to_string(reinterpret_cast<uintptr_t>(&node));
    for (size_t i = 0; i < node.stmts.size(); ++i) {
        file << "subgraph cluster_" << ident << i << " {\n";
        file << "label=\"Expression " << i << "\";\n";
        node.stmts[i]->accept(*this);
        file << "}\n";
    };
}

void TreeVisitor::render() {
    file << "}\n";
    file.flush();
    file.close();
    int result = std::system(
        "/nix/store/lvx8m4mh18aiqlqp5hdjpg3b34a07p35-graphviz-12.2.0/bin/dot "
        "-Tpng ./build/ast.dot -o ./build/ast.png");

    if (result != 0) {
        std::cerr << "Error: Graphviz failed to generate ./build/ast.png"
                  << std::endl;
    }

    std::cout << std::endl
              << "Viewing with feh. Press ESC to exit." << std::endl;
    std::cout.flush();
    int r = std::system("feh ./build/ast.png");
    std::cout << "---" << std::endl;
    if (r != 0)
        std::cerr << "Error viewing with feh" << std::endl;
}
} // namespace ast
