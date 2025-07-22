#include "run.h"

#include "tachyon/codegen/bytecode_generator.h"
#include "tachyon/lexer/lexer.h"
#include "tachyon/parser/parser.h"
#include "tachyon/parser/print.h"
#include "tachyon/runtime/vm.h"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <print>
#include <string>

using namespace tachyon;

int run(char *fileName) {
    // if bad file...
    if (!std::filesystem::exists(fileName) || !std::filesystem::is_regular_file(fileName)) {
        std::println(std::cerr, "specified file ({}) does not exist or is a directory.", fileName);
        return 1;
    }

    // load file contents
    std::ifstream file(fileName, std::ios::binary);
    std::string file_contents(std::istreambuf_iterator<char>{file}, {});

    // pipeline
    auto lexer = lexer::lex(file_contents);
    if (!lexer.errors.empty()) {
        Error e = Error::createMultiple(std::move(lexer.errors));
        e.source = file_contents;
        std::cerr << e << std::flush;
        return 1;
    }
    auto m = parser::parse(std::move(lexer.tokens), std::move(lexer.constants))
                 .and_then(codegen::generateProto)
                 .and_then([](const runtime::Proto &proto) -> std::expected<void, Error> {
                     runtime::VM vm;
                     return vm.run(proto);
                 });

    if (!m) {
        Error e = m.error();
        e.source = file_contents;
        std::cerr << e << std::flush;
        return 1;
    }

    return 0;
};
