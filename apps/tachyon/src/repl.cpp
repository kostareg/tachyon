#include "repl.h"

#include "tachyon/codegen/bytecode_generator.h"
#include "tachyon/common/error.h"
#include "tachyon/lexer/lexer.h"
#include "tachyon/parser/parser.h"

#include <iostream>
#include <string>

using namespace tachyon;

int repl() {
    runtime::VM vm;
    std::string source, line;
    std::string prefix = "> ";

    while (std::print("{}", prefix), std::getline(std::cin, line)) {
        if (line.ends_with(";;")) {
            // add the line to the source, without the ;;.
            source += line.substr(0, line.size() - 2) + '\n';
            prefix = "  ";
            continue;
        }

        // TODO: we could have a different mode of the VM for the REPL, that way we don't have to
        //  return 0 each time. Could also be return void.
        source += line + "\nreturn 0;";

        if (source.empty()) continue;

        // pipeline
        auto lexer = lexer::lex(source);
        if (!lexer.errors.empty()) {
            Error e = Error::createMultiple(std::move(lexer.errors));
            e.source = source;
            std::cerr << e << std::flush;
            return 1;
        }
        auto m = parser::parse(std::move(lexer.tokens), std::move(lexer.constants))
                     .and_then(codegen::generateProto)
                     .and_then([&vm](const runtime::Proto &proto) -> std::expected<void, Error> {
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
