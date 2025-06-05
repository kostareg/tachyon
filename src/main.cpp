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
    std::vector<uint8_t> bytecode = {
        NOOP,          // no-op --------------------------------------
        LOCR, 0, 1,    // load ABCDEF to reg 1
        LOCR, 1, 3,    // load `123` to register 3
        CREC, 3, 2, 0, // comparison
        MACR, 2, 3, 0, // math
        MSRR, 0, 3, 5, // math
        EXIT};
    std::vector<Value> constants;
    const char *x = "ABCDEF";
    constants.emplace_back(x);
    constants.emplace_back(123.0);
    constants.emplace_back(123.2);
    constants.emplace_back(0.0);
    VM vm;
    Proto mainProto{std::move(bytecode), std::move(constants)};
    auto ex = vm.run(mainProto);
    // unwrap(ex, "AAAAAAAAAAAAAAAAAAAAAAA\nBBBBBBBBBBBBBB\n", false);
    if (!ex) {
        std::println("failed with error: {}", ex.error().getSource());
    }

    vm.diagnose();
    std::println("size: {}", sizeof(constants[0]));

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
