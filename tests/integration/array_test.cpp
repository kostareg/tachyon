#include "tachyon/codegen/bytecode_generator.h"
#include "tachyon/lexer/lexer.h"
#include "tachyon/parser/parser.h"

#include <gtest/gtest.h>

#include <string>

using namespace tachyon;

static const std::string _0 = "x = [1, 2, 3]; return 0;";

TEST(ArrayTest, ConstructsWithlet)
{
    lexer::lex(_0)
        .and_then(parser::parse)
        .and_then(codegen::generateProto)
        .and_then([](const runtime::Proto &proto) -> std::expected<void, Error>
                  { return runtime::VM{}.run(proto); })
        .value();
}
