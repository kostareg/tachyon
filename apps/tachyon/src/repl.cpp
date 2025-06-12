#include "repl.h"

#include "tachyon/common/error.h"
#include "tachyon/lexer/lexer.h"
#include "tachyon/parser/parser.h"

#include <iostream>
#include <string>

int repl()
{
    vm::VM vm;
    std::string source, line;
    auto prefix = "> ";

    while (std::printf("%s", prefix) && std::getline(std::cin, line))
    {
        if (line.ends_with(";;"))
        {
            // add the line to the source, without the ;;.
            source += line.substr(0, line.size() - 2) + '\n';
            prefix = "  ";
            continue;
        }

        source += line + '\n';

        if (source.empty())
            continue;

        // pipeline
        auto m = tachyon::lexer::lex(source)
                     .and_then(parser::parse)
                     .and_then(ast::print)
                     .and_then(ast::generateProto)
                     .and_then([&vm](vm::Proto proto) -> std::expected<void, Error>
                               { return vm.run(proto); });

        if (!m)
        {
            Error e = m.error();
            e.source = source;
            std::cerr << e << std::flush;
        }

        source = "";
        prefix = "> ";
    }

    return 0;
}
