#include "visitor.hpp"
#include "ast.hpp"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <cstdlib>

template <typename T>
std::unique_ptr<T> clone_unique(const std::unique_ptr<T>& ptr) {
    return ptr ? ptr->clone() : nullptr;
}

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

void PrintVisitor::visit(VariableDeclNode& node) {
    std::cout << node.name << " = ";
    node.decl->accept(*this);
}

void PrintVisitor::visit(VariableRefNode& node) {
    std::cout << node.name;
}

void PrintVisitor::visit(FunctionDefNode& node) {
    std::cout << "def " << node.name << "(";
    std::for_each(node.args.begin(), node.args.end(), [](std::string& arg) {
        std::cout << arg << ",";
    });
    std::cout << ") {";
    node.body->accept(*this);
    std::cout << "}";
}

void PrintVisitor::visit(FunctionCallNode& node) {
    std::cout << node.name << "(";
    std::for_each(node.args.begin(), node.args.end(), [this](std::unique_ptr<ASTNode>& arg) {
        if (arg) arg->accept(*this);
        std::cout << ",";
    });
    std::cout << ")";
}

void PrintVisitor::visit(SequenceNode& node) {
    std::for_each(node.stmts.begin(), node.stmts.end(), [this](std::unique_ptr<ASTNode>& stmt) {
        if (stmt) stmt->accept(*this);
        std::cout << ";" << std::endl;
    });
}

void TreeVisitor::visit(NumberNode& node) {
    auto ident = std::to_string(reinterpret_cast<uintptr_t>(&node));
    file << ident << " [label=\"" << node.value << "\"];\n";
    if (!root.empty()) file << root << " -> " << ident << ";\n";
}

void TreeVisitor::visit(BinaryOperatorNode& node) {
    auto rootO = root;
    auto ident = std::to_string(reinterpret_cast<uintptr_t>(&node));
    file << ident << " [label=\"" << op_to_string(node.op) << "\"];\n";
    if (!root.empty()) file << root << " -> " << ident << ";\n";

    // change the root for lhs and rhs, then set it back to original.
    root = ident;

    node.left->accept(*this);
    node.right->accept(*this);

    root = rootO;
}

void TreeVisitor::visit(VariableDeclNode& node) {
    auto rootO = root;
    auto ident = std::to_string(reinterpret_cast<uintptr_t>(&node));
    file << ident << " [label=\"" << node.name << "\"];\n";
    if (!root.empty()) file << root << " -> " << ident << ";\n";

    root = ident;

    node.decl->accept(*this);

    root = rootO;
}

void TreeVisitor::visit(VariableRefNode& node) {
    auto ident = std::to_string(reinterpret_cast<uintptr_t>(&node));
    file << ident << " [label=\"" << node.name << "\"];\n";
    if (!root.empty()) file << root << " -> " << ident << ";\n";
}

void TreeVisitor::visit(FunctionDefNode& node) {
    auto rootO = root;
    auto ident = std::to_string(reinterpret_cast<uintptr_t>(&node));
    file << ident << " [label=\"" << node.name << "(";
    std::for_each(node.args.begin(), node.args.end(), [this](std::string& arg) {
        file << arg << ",";
    });
    file << ")\"];\n";
    if (!root.empty()) file << root << " -> " << ident << ";\n";

    root = ident;

    node.body->accept(*this);

    root = rootO;
}

void TreeVisitor::visit(FunctionCallNode& node) {
    auto rootO = root;
    auto ident = std::to_string(reinterpret_cast<uintptr_t>(&node));
    file << ident << " [label=\"" << node.name << "()\"];\n";
    if (!root.empty()) file << root << " -> " << ident << ";\n";

    root = ident;

    std::for_each(node.args.begin(), node.args.end(), [this](std::unique_ptr<ASTNode>& arg) {
        arg->accept(*this);
        std::cout << ",";
    });

    root = rootO;
}

void TreeVisitor::visit(SequenceNode& node) {
    auto ident = std::to_string(reinterpret_cast<uintptr_t>(&node));
    for (size_t i=0; i<node.stmts.size(); ++i) {
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
    int result = std::system("/nix/store/lvx8m4mh18aiqlqp5hdjpg3b34a07p35-graphviz-12.2.0/bin/dot -Tpng ./build/ast.dot -o ./build/ast.png");

    if (result != 0) {
        std::cerr << "Error: Graphviz failed to generate ./build/ast.png" << std::endl;
    }

    std::cout << std::endl << "Viewing with feh. Press ESC to exit." << std::endl;
    std::cout.flush();
    int r = std::system("feh ./build/ast.png");
    std::cout << "---" << std::endl;
    if (r != 0) std::cerr << "Error viewing with feh" << std::endl;
}

void OptimizationVisitor1::visit(NumberNode& node) {
    if (root) return;
    optimizedNode = std::make_unique<NumberNode>(node.value);
}

void OptimizationVisitor1::visit(BinaryOperatorNode& node) {
    if (root) return;
    root = false;
    node.left->accept(*this);
    auto left = std::move(optimizedNode);
    node.right->accept(*this);
    auto right = std::move(optimizedNode);

    // apply constant folding if both hands are NumberNodes.
    if (auto leftNum = dynamic_cast<NumberNode*>(left.get())) {
        if (auto rightNum = dynamic_cast<NumberNode*>(right.get())) {
            double result = computeBinaryOp(node.op, leftNum->value, rightNum->value);
            optimizedNode = std::make_unique<NumberNode>(result);
            return;
        }
    }

    // could not optimize, return optimized children.
    optimizedNode = std::make_unique<BinaryOperatorNode>(node.op, std::move(left), std::move(right));
}

void OptimizationVisitor1::visit(VariableDeclNode& node) {
    root = false;
    node.decl->accept(*this);
    optimizedNode = std::make_unique<VariableDeclNode>(node.name, std::move(optimizedNode));
}

void OptimizationVisitor1::visit(VariableRefNode& node) {
    varsReferenced.push_back(node.name);
    optimizedNode = std::make_unique<VariableRefNode>(node.name);
}

void OptimizationVisitor1::visit(FunctionDefNode& node) {
    root = true;
    node.body->accept(*this);
    if (optimizedNode) optimizedNode = std::make_unique<FunctionDefNode>(node.name, std::move(node.args), std::move(optimizedNode));
}

void OptimizationVisitor1::visit(FunctionCallNode& node) {
    root = false;
    std::vector<std::unique_ptr<ASTNode>> args;
    std::for_each(node.args.begin(), node.args.end(), [this, &args](std::unique_ptr<ASTNode>& arg) {
        arg->accept(*this);
        args.push_back(std::move(optimizedNode));
    });
    optimizedNode = std::make_unique<FunctionCallNode>(node.name, std::move(args));
}

void OptimizationVisitor1::visit(SequenceNode& node) {
    std::vector<std::unique_ptr<ASTNode>> stmts;
    std::for_each(node.stmts.begin(), node.stmts.end(), [this, &stmts](std::unique_ptr<ASTNode>& stmt) {
        root = true;
        stmt->accept(*this);
        if (optimizedNode) stmts.push_back(std::move(optimizedNode));
    });
    optimizedNode = std::make_unique<SequenceNode>(std::move(stmts));
}

int OptimizationVisitor1::computeBinaryOp(Op op, int left, int right) {
    switch (op) {
        case Op::Add: return left + right;
        case Op::Sub: return left - right;
        case Op::Mul: return left * right;
        case Op::Div: return left / right;
        case Op::Pow: return pow(left, right);
        default: throw std::runtime_error("unknown operator");
    }
}

void OptimizationVisitor2::visit(NumberNode& node) {
    optimizedNode = std::make_unique<NumberNode>(node.value);
};

void OptimizationVisitor2::visit(BinaryOperatorNode& node) {
    node.left->accept(*this);
    auto left = std::move(optimizedNode);
    node.right->accept(*this);
    auto right = std::move(optimizedNode);
    optimizedNode = std::make_unique<BinaryOperatorNode>(node.op, std::move(left), std::move(right));
};

void OptimizationVisitor2::visit(VariableDeclNode& node) {
    // if unused, skip.
    if (find(varsReferenced.begin(), varsReferenced.end(), node.name) == varsReferenced.end()) {
        return;
    }

    node.decl->accept(*this);
    optimizedNode = std::make_unique<VariableDeclNode>(node.name, std::move(optimizedNode));
};

void OptimizationVisitor2::visit(VariableRefNode& node) {
    optimizedNode = std::make_unique<VariableRefNode>(node.name);
};

void OptimizationVisitor2::visit(FunctionDefNode& node) {
    node.body->accept(*this);
    optimizedNode = std::make_unique<FunctionDefNode>(node.name, std::move(node.args), std::move(optimizedNode));
};

void OptimizationVisitor2::visit(FunctionCallNode& node) {
    std::vector<std::unique_ptr<ASTNode>> args;
    std::for_each(node.args.begin(), node.args.end(), [this, &args](std::unique_ptr<ASTNode>& arg) {
        arg->accept(*this);
        args.push_back(std::move(optimizedNode));
    });
    optimizedNode = std::make_unique<FunctionCallNode>(node.name, std::move(args));
};

void OptimizationVisitor2::visit(SequenceNode& node) {
    std::vector<std::unique_ptr<ASTNode>> stmts;
    std::for_each(node.stmts.begin(), node.stmts.end(), [this, &stmts](std::unique_ptr<ASTNode>& stmt) {
        stmt->accept(*this);
        if (optimizedNode) stmts.push_back(std::move(optimizedNode));
    });
    optimizedNode = std::make_unique<SequenceNode>(std::move(stmts));
};

void LoweringVisitor::visit(NumberNode& node) {
    tmp = std::make_unique<ir::NumberNode>(node.value);
}

void LoweringVisitor::visit(BinaryOperatorNode& node) {
    node.left->accept(*this);
    auto left = std::move(tmp);
    node.right->accept(*this);
    auto right = std::move(tmp);

    auto tx = tmpVar();

    loweredNodes.push_back(std::make_unique<ir::VariableDeclNode>(tx, std::make_unique<ir::BinaryOperatorNode>(node.op, std::move(left), std::move(right))));
    tmp = std::make_unique<ir::VariableRefNode>(tx);
}

void LoweringVisitor::visit(VariableDeclNode& node) {
    node.decl->accept(*this);
    loweredNodes.push_back(std::make_unique<ir::VariableDeclNode>(node.name, std::move(tmp)));
}

void LoweringVisitor::visit(VariableRefNode& node) {
    tmp = std::make_unique<ir::VariableRefNode>(node.name);
}

void LoweringVisitor::visit(FunctionDefNode& node) {
    // clear the lowered nodes list, copy everything into the function node,
    // then bring back lowered nodes list.
    auto otherNodes = std::move(loweredNodes);
    node.body->accept(*this);
    otherNodes.push_back(std::make_unique<ir::BlockNode>(node.name, std::make_unique<ir::SequenceNode>(std::move(loweredNodes))));
    loweredNodes = std::move(otherNodes);
}

void LoweringVisitor::visit(FunctionCallNode& node) {
    std::for_each(node.args.begin(), node.args.end(), [this](std::unique_ptr<ASTNode>& arg) {
        arg->accept(*this);
        if (tmp) loweredNodes.push_back(std::make_unique<ir::ParamNode>(std::move(tmp)));
    });
    auto tx = tmpVar();
    loweredNodes.push_back(std::make_unique<ir::BlockCallNode>(node.name, tx));
    tmp = std::make_unique<ir::VariableRefNode>(tx);
}

void LoweringVisitor::visit(SequenceNode& node) {
    std::for_each(node.stmts.begin(), node.stmts.end(), [this](std::unique_ptr<ASTNode>& stmt) {
        stmt->accept(*this);
        if (tmp) loweredNodes.push_back(std::move(tmp));
    });
}

