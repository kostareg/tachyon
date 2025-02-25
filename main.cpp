#include "parser.hpp"
#include "ast.hpp"

//
// main is where all program execution starts
//
int main(int argc, char *argv[]) {
    auto expr = std::make_unique<BinaryOperatorNode>(
        Op::Add,
        std::make_unique<BinaryOperatorNode>(
            Op::Add,
            std::make_unique<NumberNode>(3),
            std::make_unique<NumberNode>(5)
        ),
        std::make_unique<NumberNode>(5)
    );

    PrintVisitor printer;
    expr->accept(printer);

    TreeVisitor tree;
    expr->accept(tree);
    tree.render();

    return 0;
}
