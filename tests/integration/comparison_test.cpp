#include "tachyon/codegen/bytecode_generator.hpp"
#include "tachyon/lexer/lexer.hpp"
#include "tachyon/parser/parser.hpp"

#include <gtest/gtest.h>

#include <string>

using namespace tachyon;

static const std::string all_operators_source =
    "1 < 1; 2 > 2; x = 5; y = 2; x == 5; y < 1; return 0;";

TEST(ComparisonTest, AllOperators) {
    runtime::VM vm{};

    lexer::Lexer lexer = lexer::lex(all_operators_source);
    if (!lexer.errors.empty()) {
        Error e = Error::createMultiple(std::move(lexer.errors));
        e.source = all_operators_source;
        std::cerr << e << std::flush;
        ASSERT_EQ(0, 1);
    }
    parser::parse(std::move(lexer.tokens), std::move(lexer.constants))
        .and_then(codegen::generate_main_proto)
        .and_then([&vm](const runtime::Proto &proto) -> std::expected<void, Error> {
            return vm.run(proto);
        })
        .value();
}
