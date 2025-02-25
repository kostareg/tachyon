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

    std::vector<std::unique_ptr<ASTNode>> stmts;
    stmts.push_back(std::make_unique<FunctionDefNode>("myvar", std::vector<std::string>{"param1", "param2"}, std::make_unique<BinaryOperatorNode>(
          Op::Add,
          std::make_unique<BinaryOperatorNode>(
              Op::Mul,
              std::make_unique<BinaryOperatorNode>(
                  Op::Add,
                  std::make_unique<NumberNode>(3),
                  std::make_unique<NumberNode>(5)
              ),
              std::make_unique<VariableRefNode>("y")
        ), std::make_unique<FunctionCallNode>("myfn", std::move(exampleArgs)))));
    stmts.push_back(std::make_unique<NumberNode>(3));
    stmts.push_back(std::make_unique<BinaryOperatorNode>(
              Op::Mul,
              std::make_unique<BinaryOperatorNode>(
                  Op::Add,
                  std::make_unique<NumberNode>(3),
                  std::make_unique<NumberNode>(5)
              ),
              std::make_unique<VariableRefNode>("y")
        ));
    stmts.push_back(std::make_unique<VariableDeclNode>(
              "l",
              std::make_unique<BinaryOperatorNode>(
                  Op::Mul,
                  std::make_unique<NumberNode>(7),
                  std::make_unique<NumberNode>(7)
              )
        ));
    stmts.push_back(std::make_unique<VariableDeclNode>(
              "y",
              std::make_unique<BinaryOperatorNode>(
                  Op::Mul,
                  std::make_unique<VariableRefNode>("l"),
                  std::make_unique<NumberNode>(7)
              )
        ));
    stmts.push_back(std::make_unique<VariableDeclNode>(
              "n",
              std::make_unique<BinaryOperatorNode>(
                  Op::Mul,
                  std::make_unique<NumberNode>(7),
                  std::make_unique<NumberNode>(7)
              )
        ));
    stmts.push_back(std::make_unique<BinaryOperatorNode>(
              Op::Mul,
              std::make_unique<BinaryOperatorNode>(
                  Op::Add,
                  std::make_unique<NumberNode>(3),
                  std::make_unique<VariableRefNode>("n")
              ),
              std::make_unique<VariableRefNode>("y")
        ));

    auto expr = std::make_unique<SequenceNode>(std::move(stmts));

    TreeVisitor tree0;
    expr->accept(tree0);
    tree0.render();

    OptimizationVisitor1 opt1;
    expr->accept(opt1);

    OptimizationVisitor2 opt2(opt1.varsReferenced);
    opt1.optimizedNode->accept(opt2);

    PrintVisitor printer;
    opt2.optimizedNode->accept(printer);

    TreeVisitor tree1;
    opt2.optimizedNode->accept(tree1);
    tree1.render();

    return 0;
}
