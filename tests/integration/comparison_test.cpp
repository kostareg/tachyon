#include "tachyon/codegen/bytecode_generator.h"
#include "tachyon/lexer/lexer.h"
#include "tachyon/parser/parser.h"

#include <gtest/gtest.h>

#include <string>

using namespace tachyon;

static const std::string all_operators_source = "1 < 1; 2 > 2; x = 5; y = 2; x == 5; y < 1;";

TEST(ComparisonTest, AllOperators)
{
    runtime::VM vm{};

    lexer::lex(all_operators_source)
        .and_then(parser::parse)
        .and_then(codegen::generateProto)
        .and_then([&vm](const runtime::Proto &proto) -> std::expected<void, Error>
                  { return vm.run(proto); });
}
