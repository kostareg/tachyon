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

// TODO: this is temporary. currently it applies to the whole file but it should really just apply
//  to strings and that's it.
/**
 * @brief unescapes file input, so that escape chars like \n in strings works.
 * @param input content to unescape
 * @return unescaped string
 */
std::string unescape(const std::string &input) {
    std::string result;
    result.reserve(input.size());

    for (size_t i = 0; i < input.size(); ++i) {
        if (input[i] == '\\' && i + 1 < input.size()) {
            switch (input[++i]) {
            case 'n': result += '\n'; break;
            case 't': result += '\t'; break;
            case 'r': result += '\r'; break;
            case '\\': result += '\\'; break;
            case '"': result += '"'; break;
            case '\'': result += '\''; break;
            case '0':
                result += '\0';
                break;
                // Add more cases if needed
            default: result += '\\'; result += input[i];
            }
        } else {
            result += input[i];
        }
    }

    return result;
}

int run(char *fileName) {
    // if bad file...
    if (!std::filesystem::exists(fileName) || !std::filesystem::is_regular_file(fileName)) {
        std::println(std::cerr, "specified file ({}) does not exist or is a directory.", fileName);
        return 1;
    }

    // load file contents
    std::ifstream file(fileName, std::ios::binary);
    std::string file_contents(std::istreambuf_iterator<char>{file}, {});
    file_contents = unescape(file_contents);

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
