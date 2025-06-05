#include <cstring>
#include <expected>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <ostream>
#include <string>
#include <variant>

import parser;
import ast;
import lexer;
import vm;
import error;

void unwrap(std::expected<void, Error> t, const std::string &src, bool quit) {
    if (!t.has_value()) {
        auto e = t.error();
        constexpr auto message = "\033[31merror{}\033[0m: {} on L{}C{}.\n"
                                 "{}";

        auto code = e.code.empty() ? "" : "(" + e.code + ")";

        // TODO: for now:
        e.source = get_line_at(src, e.span.pos);

        std::cout << std::format(message, code, e.messageLong, e.span.line,
                                 e.span.column, e.getSource());

        for (auto &i : e.additional) {
            unwrap(std::unexpected(i), src, false);
        }

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

    // TODO: update this with other stuff
    auto m = lexer::lex(file_contents)
                 .and_then(parser::parse)
                 // .and_then(optimize)
                 .and_then(ast::generate_proto)
                 .and_then([](vm::Proto proto) -> std::expected<void, Error> {
                     vm::VM vm;
                     return vm.run(proto);
                 });

    unwrap(m, file_contents, true);

    return 0;
}

int repl() {
    vm::VM vm;
    std::string source, line;
    auto prefix = "> ";

    while (std::printf("%s", prefix) && std::getline(std::cin, line)) {
        if (line.ends_with(";;")) {
            // add the line to the source, without the ;;.
            source += line.substr(0, line.size() - 2) + '\n';
            prefix = "  ";
            continue;
        }

        source += line + '\n';

        if (source.empty())
            continue;

        auto m =
            lexer::lex(source)
                .and_then(parser::parse)
                .and_then(ast::print)
                .and_then(ast::infer)
                .and_then(ast::print)
                .and_then(ast::generate_proto)
                .and_then([&vm](vm::Proto proto) -> std::expected<void, Error> {
                    return vm.run(proto);
                });

        unwrap(m, source, false);

        source = "";
        prefix = "> ";
    }

    return 0;
}

using namespace vm;

int testvm() {
    // --- define myfn --- //
    std::vector<uint8_t> bytecode_myfn = {
        NOOP,          // no-op --------------------------------------
        MARR, 0, 1, 0, // add reg0 + reg1 -> reg0
        MARC, 0, 1, 0, // add reg0 + 1 -> reg0
        RETR, 0};
    std::vector<Value> constants_myfn;
    constants_myfn.emplace_back("ZYXWVU");
    constants_myfn.emplace_back(1.0);
    Proto myfnProto{std::move(bytecode_myfn), std::move(constants_myfn), 2,
                    "myfn"};

    // --- define main --- //
    // can also CALC 4 directly
    std::vector<uint8_t> bytecode_main = {
        LOCR, 0,  1,     // load `ABCDEF` to reg 1
        LOCR, 1,  3,     // load `123` to register 3
        CREC, 3,  2,  0, // comparison
        MACR, 2,  3,  0, // math
        MSRR, 0,  3,  5, // math
        LOCR, 1,  1,     // load `123` to register 1
        LOCR, 4,  10,    // load myfn to reg10
        CALR, 10,        // call reg10
        RETV};
    std::vector<Value> constants_main;
    const char *x = "ABCDEF";
    constants_main.emplace_back(x);
    constants_main.emplace_back(123.0);
    constants_main.emplace_back(123.2);
    constants_main.emplace_back(0.0);
    constants_main.emplace_back(&myfnProto);
    Proto mainProto{std::move(bytecode_main), std::move(constants_main), 0,
                    "main"};

    // vm
    VM vm;
    if (auto ex = vm.run(mainProto); !ex) {
        std::println("failed with error: {}", ex.error().getSource());
    }
    // unwrap(ex, "AAAAAAAAAAAAAAAAAAAAAAA\nBBBBBBBBBBBBBB\n", false);

    vm.diagnose();
    std::println("size: {}", sizeof(constants_main[0]));

    return 0;
}

int main(int argc, char **argv) {
    if (argc == 1) {
        return repl();
    } else if (strcmp(argv[1], "run") == 0) {
        if (argc == 2) {
            std::println(std::cerr, "specify a file to run.\n> "
                                    "tachyon run myfile.tachyon");
            return 1;
        }
        return run(argv[2]);
    } else if (strcmp(argv[1], "testvm") == 0) {
        return testvm();
    } else {
        std::println(std::cerr, "unknown command: {}", argv[1]);
        return 1;
    }
}
