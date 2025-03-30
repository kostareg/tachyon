#include <iostream>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include "lexer/lexer.hpp"
#include "parser/parser.hpp"

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

    std::ifstream file("examples/one.tachyon");
    std::string str;
    std::string file_contents;
    while (std::getline(file, str)) {
        file_contents += str;
        file_contents.push_back('\n');
    }

    lexer::Lexer t;
    auto nodes = t.lex(file_contents)
                     .and_then([](std::vector<Token> toks) {
                         parser::Parser p(toks);
                         return p.parse();
                     })
                     .unwrap_with_src(file_contents);

    /*
    std::cout << "------------" << std::endl;
    for (int i = 0; i < toks.size(); ++i) {
        toks[i].print();
        std::cout << std::endl;
    }
    std::cout << "------------" << std::endl;
    */

    std::cout << "done parsing" << std::endl;

    ast::PrintVisitor pr;
    nodes->accept(pr);

    ast::OptimizationVisitor1 opt1;
    nodes->accept(opt1);

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

    lower2.finish();

    std::cout << "------vm------" << std::endl;

    vm::VM vm1;
    vm1.run_fn(lower2.proto.get());

    std::cout << "--------------" << std::endl;

    return 0;
}
