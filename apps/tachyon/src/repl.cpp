#include "repl.hpp"

#include "tachyon/codegen/bytecode_generator.hpp"
#include "tachyon/common/error.hpp"
#include "tachyon/lexer/lexer.hpp"
#include "tachyon/parser/parser.hpp"

#include <iostream>
#include <string>

using namespace tachyon;

int repl() {
    runtime::VM vm;
    vm.set_mode(runtime::Mode::Repl);
    std::string source, line;
    std::string prefix = "> ";

    while (std::print("{}", prefix), std::getline(std::cin, line)) {
        if (line.ends_with(";;")) {
            // add the line to the source, without the ;;.
            source += line.substr(0, line.size() - 2) + '\n';
            prefix = "  ";
            continue;
        }

        if (line.empty()) continue;

        source += line + "\n";

        // pipeline
        auto lexer = lexer::lex(source);
        if (!lexer.errors.empty()) {
            Error e = Error::createMultiple(std::move(lexer.errors));
            e.source = source;
            std::cerr << e << std::flush;
            return 1;
        }
        auto m = parser::parse(std::move(lexer.tokens), std::move(lexer.constants))
                     .and_then(codegen::generate_repl_proto)
                     .and_then([&vm](runtime::Proto proto) -> std::expected<void, Error> {
                         return vm.run(proto);
                     });

        if (!m) {
            Error e = m.error();
            e.source = source;
            std::cerr << e << std::flush;
        }

        source = "";
        prefix = "> ";
    }

    return 0;
}
