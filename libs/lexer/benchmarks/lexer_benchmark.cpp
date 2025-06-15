#include "tachyon/lexer/lexer.h"

#include <benchmark/benchmark.h>

static const std::string basic_source = R"(fn add(x, y) { return x + y; })";

static void BM_BasicLex(benchmark::State &state)
{
    for (auto _ : state)
    {
        auto tokens = tachyon::lexer::lex(basic_source);
        benchmark::DoNotOptimize(tokens);
    }
}

BENCHMARK(BM_BasicLex)->MinTime(1.0);

BENCHMARK_MAIN();
