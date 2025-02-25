#include "parser.hpp"
#include "ast.hpp"
#include <iostream>

//
// main is where all program execution starts
//
int main(int argc, char *argv[]) {
    auto expr = std::make_unique<VariableDeclNode>("myvar", std::make_unique<BinaryOperatorNode>(
        Op::Mul,
        std::make_unique<BinaryOperatorNode>(
            Op::Add,
            std::make_unique<NumberNode>(3),
            std::make_unique<NumberNode>(5)
        ),
        std::make_unique<VariableRefNode>("y")
    ));

    //TreeVisitor tree1;
    //expr->accept(tree1);
    //tree1.render();

    OptimizationVisitor opt;
    expr->accept(opt);

    PrintVisitor printer;
    opt.optimizedNode->accept(printer);

    TreeVisitor tree1;
    opt.optimizedNode->accept(tree1);
    tree1.render();

    return 0;
}
