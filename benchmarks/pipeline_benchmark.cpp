#include "tachyon/codegen/bytecode_generator.hpp"
#include "tachyon/lexer/lexer.hpp"
#include "tachyon/parser/parser.hpp"
#include "tachyon/runtime/vm.hpp"

#include <benchmark/benchmark.h>

using namespace tachyon;

static const std::string basic_source = "x = 1; y = 2; return x + y;";

static void BM_BasicPipeline(benchmark::State &state) {
    for (auto _ : state) {
        lexer::Lexer lexer = lexer::lex(basic_source);
        parser::parse(std::move(lexer.tokens), std::move(lexer.constants))
            .and_then(codegen::generate_main_proto)
            .and_then([](runtime::Proto proto) -> std::expected<void, Error> {
                return runtime::VM().run(proto);
            });
    }
}

BENCHMARK(BM_BasicPipeline);

BENCHMARK_MAIN();
