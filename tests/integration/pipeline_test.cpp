#include "tachyon/codegen/bytecode_generator.hpp"
#include "tachyon/lexer/lexer.hpp"
#include "tachyon/parser/parser.hpp"

#include <gtest/gtest.h>

#include <string>

using namespace tachyon;

static const std::string basic_source = "1 + 1; return 0;";

TEST(PipelineTest, BasicPipeline) {
    lexer::Lexer lexer = lexer::lex(basic_source);
    if (!lexer.errors.empty()) {
        Error e = Error::createMultiple(std::move(lexer.errors));
        e.source = basic_source;
        std::cerr << e << std::flush;
        ASSERT_EQ(0, 1);
    }
    parser::parse(lexer.tokens, lexer.constants)
        .and_then(codegen::generate_main_proto)
        .and_then([](const runtime::Proto &proto) -> std::expected<void, Error> {
            return runtime::VM().run(proto);
        })
        .value();
}
