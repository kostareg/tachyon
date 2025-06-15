#include "tachyon/codegen/bytecode_generator.h"
#include "tachyon/lexer/lexer.h"
#include "tachyon/parser/parser.h"

#include <gtest/gtest.h>

#include <string>

using namespace tachyon;

static const std::string basic_source = "1 + 1;";

TEST(PipelineTest, BasicPipeline)
{
    lexer::lex(basic_source)
        .and_then(parser::parse)
        .and_then(codegen::generateProto)
        .and_then([](const runtime::Proto &proto) -> std::expected<void, Error>
                  { return runtime::VM().run(proto); });
}