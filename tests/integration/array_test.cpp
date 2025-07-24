#include "tachyon/codegen/bytecode_generator.hpp"
#include "tachyon/lexer/lexer.hpp"
#include "tachyon/parser/parser.hpp"

#include <gtest/gtest.h>

#include <string>

using namespace tachyon;

static const std::string _0 = "x = [1, 2, 3]; return 0;";

TEST(ArrayTest, ConstructsWithlet) {
    lexer::Lexer lexer = lexer::lex(_0);
    if (!lexer.errors.empty()) {
        Error e = Error::createMultiple(std::move(lexer.errors));
        e.source = _0;
        std::cerr << e << std::flush;
        ASSERT_EQ(0, 1);
    }
    parser::parse(std::move(lexer.tokens), std::move(lexer.constants))
        .and_then(codegen::generate_main_proto)
        .and_then([](const runtime::Proto &proto) -> std::expected<void, Error> {
            return runtime::VM{}.run(proto);
        })
        .value();
}
