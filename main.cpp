#include "parser.hpp"
#include "ast.hpp"
#include <iostream>

//
// main is where all program execution starts
//
int main(int argc, char *argv[]) {
    auto expr = std::make_unique<BinaryOperatorNode>(
        Op::Mul,
        std::make_unique<BinaryOperatorNode>(
            Op::Add,
            std::make_unique<NumberNode>(3),
            std::make_unique<NumberNode>(5)
        ),
        std::make_unique<NumberNode>(5)
    );

    TreeVisitor tree1;
    expr->accept(tree1);
    tree1.render();

    OptimizationVisitor opt;
    expr->accept(opt);

    std::cout << "done opt" << std::endl;

    PrintVisitor printer;
    opt.optimizedNode->accept(printer);

    return 0;
}
