#include <iostream>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include "ast/ast.hpp"
#include "parser/tokenizer.hpp"
#include "vm/vm.hpp"

int main(int argc, char *argv[]) {
#ifdef DEBUG
    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    console_sink->set_level(spdlog::level::trace); // Enable trace messages
    console_sink->set_pattern("%^%Y-%m-%d %H:%M:%S.%e [%L] %v%$");

    // Create a logger with the colored sink
    auto logger = std::make_shared<spdlog::logger>("colored_logger", console_sink);
    spdlog::set_default_logger(logger);

    // Set global log level to trace
    spdlog::set_level(spdlog::level::trace);
#endif

    /*
    std::vector<std::unique_ptr<ast::ASTNode>> mainS;
    std::vector<std::unique_ptr<ast::ASTNode>> fn1S;
    std::vector<std::unique_ptr<ast::ASTNode>> fn2S;

    // x = 10
    fn1S.push_back(
        std::make_unique<ast::VariableDeclNode>("x", std::make_unique<ast::NumberNode>(10)));
    // y = x
    fn1S.push_back(
        std::make_unique<ast::VariableDeclNode>("y", std::make_unique<ast::VariableRefNode>("x")));
    // z = x * y
    fn1S.push_back(std::make_unique<ast::VariableDeclNode>(
        "z", std::make_unique<ast::BinaryOperatorNode>(
                 Op::Mul, std::make_unique<ast::VariableRefNode>("x"),
                 std::make_unique<ast::VariableRefNode>("y"))));
    std::vector<std::unique_ptr<ast::ASTNode>> fn1SPrintArgs;
    fn1SPrintArgs.push_back(std::make_unique<ast::VariableRefNode>("z"));
    // print z
    fn1S.push_back(std::make_unique<ast::FunctionCallNode>("print", std::move(fn1SPrintArgs)));
    auto fn1A = ast::SequenceNode(std::move(fn1S));

    ast::PrintVisitor printA0;
    fn1A.accept(printA0);

    ast::OptimizationVisitor1 opt1;
    fn1A.accept(opt1);

    std::cout << "ast opt 1:" << std::endl;
    ast::PrintVisitor printA1;
    opt1.optimizedNode->accept(printA1);

    ast::OptimizationVisitor2 opt2(opt1.varsReferenced);
    opt1.optimizedNode->accept(opt2);

    std::cout << "ast opt 2:" << std::endl;
    ast::PrintVisitor printA2;
    opt2.optimizedNode->accept(printA2);

    ast::LoweringVisitor lower1;
    opt2.optimizedNode->accept(lower1);
    auto n = std::make_unique<ir::SequenceNode>(std::move(lower1.loweredNodes));

    ir::PrintVisitor printI;
    n->accept(printI);

    ir::LoweringVisitor lower2("main");
    n->accept(lower2);

    for (int i = 0; i < 20; ++i) {
        std::cout << i << " : " << "0x" << std::hex << std::setw(4) << std::setfill('0')
                  << lower2.program[i] << std::dec << std::endl;
    }
    // TODO: prints 10 not 100

    lower2.finish();

    spdlog::trace("---------------------------------");

    vm::VM vm1;
    vm1.run_fn(lower2.proto.get());

    spdlog::trace("---------------------------------");

    // ---

    uint16_t fn1[0xFFFF] = {};
    fn1[0] = 0x0001;
    fn1[1] = 0x0010;
    fn1[2] = 0;
    fn1[3] = 10;
    fn1[4] = 0x0011;
    fn1[5] = 1;
    fn1[6] = 0;
    fn1[7] = 0x005C;
    fn1[8] = 0;
    fn1[9] = 1;
    fn1[10] = 0;
    fn1[11] = 0x0000;

    uint16_t fn2[0xFFFF] = {};
    fn2[0] = 0x0001;
    fn2[1] = 0x005A;
    fn2[2] = 0;
    fn2[3] = 1;
    fn2[4] = 0;
    fn2[5] = 0x0004;
    fn2[6] = 0;

    uint16_t main[0xFFFF] = {};
    main[0] = 0x0001;
    main[1] = 0x0100;
    main[2] = 0;
    main[3] = 0;
    main[4] = 0x0110;
    main[5] = 0;
    main[6] = 0x0001;
    main[7] = 0x0120;
    main[8] = 0;
    main[9] = 10;
    main[10] = 0x0120;
    main[11] = 1;
    main[12] = 20;
    main[13] = 0x0100;
    main[14] = 2;
    main[15] = 1;
    main[16] = 0x0110;
    main[17] = 2;
    main[18] = 0x0100;
    main[19] = 1;
    main[20] = 0xFFFF;
    main[21] = 0x0121;
    main[22] = 0;
    main[23] = 2;
    main[24] = 0x0110;
    main[25] = 1;
    main[26] = 0x0000;

    std::vector<std::unique_ptr<vm::Proto>> a;
    a.push_back(std::make_unique<vm::Proto>("fn1", fn1));
    a.push_back(std::make_unique<vm::Proto>("fn2", fn2, 2, 1));
    vm::Proto main_proto("main", main, std::move(a));

    vm::VM vm;
    vm.run_fn(&main_proto);
    // std::cout << vm.registers[0] << std::endl;
    // std::cout << vm.registers[1] << std::endl;
    */

    std::ifstream file("examples/one.tachyon");
    std::string str;
    std::string file_contents;
    while (std::getline(file, str)) {
        file_contents += str;
        file_contents.push_back('\n');
        std::cout << str << std::endl;
    }

    Tokenizer t;
    auto toks = t.tokenize(file_contents);

    std::cout << "------------" << std::endl;
    for (int i = 0; i < toks.size(); ++i) {
        toks[i].print();
        std::cout << std::endl;
    }

    return 0;
}
