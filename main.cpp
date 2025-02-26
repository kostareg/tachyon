#include "parser.hpp"
#include "ast.hpp"
#include "vm.hpp"
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

    std::vector<std::unique_ptr<ASTNode>> myfnBodyArg;
    myfnBodyArg.push_back(std::make_unique<VariableRefNode>("o"));

    std::vector<std::unique_ptr<ASTNode>> myfnBody;
    myfnBody.push_back(std::make_unique<VariableDeclNode>("o", std::make_unique<BinaryOperatorNode>(
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
    myfnBody.push_back(std::make_unique<FunctionCallNode>("print", std::move(myfnBodyArg)));

    std::vector<std::unique_ptr<ASTNode>> stmts;

    stmts.push_back(std::make_unique<FunctionDefNode>("myfn", std::vector<std::string>{"param1", "param2"}, std::make_unique<SequenceNode>(std::move(myfnBody))));
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

    /*
    TreeVisitor tree0;
    expr->accept(tree0);
    tree0.render();*/

    OptimizationVisitor1 opt1;
    expr->accept(opt1);

    OptimizationVisitor2 opt2(opt1.varsReferenced);
    opt1.optimizedNode->accept(opt2);

    PrintVisitor printer;
    opt2.optimizedNode->accept(printer);

    TreeVisitor tree1;
    opt2.optimizedNode->accept(tree1);
    tree1.render();

    LoweringVisitor lower;
    opt2.optimizedNode->accept(lower);

    ir::PrintVisitor printLow;
    auto seq = std::make_unique<ir::SequenceNode>(std::move(lower.loweredNodes));
    seq->accept(printLow);

      /*
    VM vm;
    uint16_t memory[0xFFFF] = {};
    memory[8] = 1;
    memory[9] = 2;
    memory[10] = 0;
    memory[11] = 1000;
    memory[12] = 3;
    memory[13] = 1;
    memory[14] = 0;
    memory[15] = 0;
    vm.load(memory);
    vm.run();
    std::cout << vm.registers[0] << std::endl;
    std::cout << vm.registers[1] << std::endl;*/

    return 0;
}
