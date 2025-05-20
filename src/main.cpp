#include <cmath>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <ostream>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>
#include <string>

#include "ast/ast.hpp"
#include "lexer/lexer.hpp"
#include "parser/parser.hpp"
#include "vm/vm.hpp"

void unwrap(std::expected<void, Error> t, std::string src, bool quit) {
    if (!t.has_value()) {
        auto e = t.error();
        constexpr auto message = "\033[31merror{}\033[0m: {} on L{}C{}.\n"
                                 "{}";

        auto code = e.code.empty() ? "" : "(" + e.code + ")";

        // TODO: for now:
        e.source = get_line_at(src, e.span.pos);

        std::cout << std::format(message, code, e.messageLong, e.span.line,
                                 e.span.column, e.getSource());
        if (quit)
            exit(1);
    }
};

// TODO: consider moving optimisations to IR.
// std::expected<ast::ExprRefs, Error> optimize(ast::ExprRefs);

int run(char *fileName) {
    // if bad file...
    if (!std::filesystem::exists(fileName) ||
        !std::filesystem::is_regular_file(fileName)) {
        std::println(std::cerr,
                     "specified file ({}) does not exist or is a directory.",
                     fileName);
        return 1;
    }

    // load file contents
    std::ifstream file(fileName, std::ios::binary);
    std::string file_contents(std::istreambuf_iterator<char>{file}, {});

    auto m = lexer::lex(file_contents)
                 .and_then(parser::parse)
                 // .and_then(optimize)
                 .and_then(ast::generate_proto)
                 .and_then([](vm::Proto proto) -> std::expected<void, Error> {
                     vm::VM vm;
                     return vm.run_fn(&proto);
                 });

    unwrap(m, file_contents, true);

    return 0;
}

int repl() {
    vm::VM vm;
    std::string source;

    while (std::printf("> ") && std::getline(std::cin, source)) {
        if (source.empty())
            continue;

        auto m =
            lexer::lex(source)
                .and_then(parser::parse)
                .and_then(ast::generate_proto)
                .and_then([&vm](vm::Proto proto) -> std::expected<void, Error> {
                    return vm.run_fn(&proto);
                });

        unwrap(m, source, false);
    }

    return 0;
}

int main(int argc, char **argv) {
#ifdef DEBUG
    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    console_sink->set_level(spdlog::level::trace); // Enable trace messages
    console_sink->set_pattern("%^%Y-%m-%d %H:%M:%S.%e [%L] %v%$");

    // Create a logger with the colored sink
    auto logger =
        std::make_shared<spdlog::logger>("colored_logger", console_sink);
    spdlog::set_default_logger(logger);

    // Set global log level to trace
    spdlog::set_level(spdlog::level::trace);
#endif

    if (argc == 1) {
        return repl();
    } else if (strcmp(argv[1], "run") == 0) {
        if (argc == 2) {
            std::println(std::cerr, "specify a file to run.\n> "
                                    "tachyon run myfile.tachyon");
            return 1;
        }
        return run(argv[2]);
    } else {
        std::println(std::cerr, "unknown command: {}", argv[1]);
        return 1;
    }
}
