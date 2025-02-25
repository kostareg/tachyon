#include "parser.hpp"
#include "ast.hpp"
#include <iostream>

//
// main is where all program execution starts
//
int main(int argc, char *argv[]) {
    std::vector<std::unique_ptr<ASTNode>> exampleArgs;
    exampleArgs.push_back(std::make_unique<BinaryOperatorNode>(Op::Add,
          std::make_unique<NumberNode>(2),
          std::make_unique<NumberNode>(4)));
    exampleArgs.push_back(std::make_unique<BinaryOperatorNode>(Op::Pow,
          std::make_unique<NumberNode>(2),
          std::make_unique<NumberNode>(4)));

    auto expr = std::make_unique<FunctionDefNode>("myvar", std::vector<std::string>{"param1", "param2"}, std::make_unique<BinaryOperatorNode>(
          Op::Add,
          std::make_unique<BinaryOperatorNode>(
              Op::Mul,
              std::make_unique<BinaryOperatorNode>(
                  Op::Add,
                  std::make_unique<NumberNode>(3),
                  std::make_unique<NumberNode>(5)
              ),
              std::make_unique<VariableRefNode>("y")
        ), std::make_unique<FunctionCallNode>("myfn", std::move(exampleArgs))));

    TreeVisitor tree0;
    expr->accept(tree0);
    tree0.render();

    OptimizationVisitor opt;
    expr->accept(opt);

    PrintVisitor printer;
    opt.optimizedNode->accept(printer);

    TreeVisitor tree1;
    opt.optimizedNode->accept(tree1);
    tree1.render();

    return 0;
}
