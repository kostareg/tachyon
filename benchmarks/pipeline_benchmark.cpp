#include "tachyon/codegen/bytecode_generator.h"
#include "tachyon/lexer/lexer.h"
#include "tachyon/parser/parser.h"
#include "tachyon/runtime/vm.h"

#include <benchmark/benchmark.h>

using namespace tachyon;

static const std::string basic_source = "1 + 1;";

static void BM_BasicPipeline(benchmark::State &state)
{
    for (auto _ : state)
    {
        lexer::lex(basic_source)
            .and_then(parser::parse)
            .and_then(codegen::generateProto)
            .and_then([](auto proto) -> std::expected<void, Error>
                      { return runtime::VM().run(proto); });
    }
}

BENCHMARK(BM_BasicPipeline);

BENCHMARK_MAIN();
