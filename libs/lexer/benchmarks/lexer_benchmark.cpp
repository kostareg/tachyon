#include "tachyon/lexer/lexer.h"

#include <benchmark/benchmark.h>
#include <fstream>
#include <string>

using namespace tachyon;

class LexerDataFixture : public benchmark::Fixture
{
  public:
    void SetUp(const ::benchmark::State &) override
    {
        const std::string included_files[] = {
            "00-blank",      "01-basic",       "02-1000-chars",  "03-2000-chars",
            "04-5000-chars", "05-10000-chars", "06-20000-chars", "07-big-comment",
        };

        for (const std::string &included_file : included_files)
        {
            const std::string included_file_path = "benchmark-data/" + included_file + ".tachyon";
            std::ifstream file(included_file_path, std::ios::binary | std::ios::ate);
            std::streamsize size = file.tellg();
            if (size == -1)
                throw std::runtime_error("could not find file: " + included_file);
            file.seekg(0, std::ios::beg);
            std::string content(size, '\0');
            file.read(content.data(), size);
            lexer_data[included_file] = content;
        }
    }

    void TearDown(const ::benchmark::State &) override { lexer_data.clear(); }

  protected:
    std::unordered_map<std::string, std::string> lexer_data;
};

BENCHMARK_F(LexerDataFixture, LexBlank)(benchmark::State &state)
{
    std::string _00_blank = lexer_data.at("00-blank");

    for (auto _ : state)
    {
        lexer::Tokens tokens = *lexer::lex(_00_blank);
        benchmark::DoNotOptimize(tokens);
    }
};

BENCHMARK_F(LexerDataFixture, LexBasic)(benchmark::State &state)
{
    std::string _01_basic = lexer_data.at("01-basic");

    for (auto _ : state)
    {
        lexer::Tokens tokens = *lexer::lex(_01_basic);
        benchmark::DoNotOptimize(tokens);
    }
};

BENCHMARK_F(LexerDataFixture, Lex1000Chars)(benchmark::State &state)
{
    std::string _02_1000_chars = lexer_data.at("02-1000-chars");

    for (auto _ : state)
    {
        lexer::Tokens tokens = *lexer::lex(_02_1000_chars);
        benchmark::DoNotOptimize(tokens);
    }
};

BENCHMARK_F(LexerDataFixture, Lex2000Chars)(benchmark::State &state)
{
    std::string _03_2000_chars = lexer_data.at("03-2000-chars");

    for (auto _ : state)
    {
        lexer::Tokens tokens = *lexer::lex(_03_2000_chars);
        benchmark::DoNotOptimize(tokens);
    }
};

BENCHMARK_F(LexerDataFixture, Lex5000Chars)(benchmark::State &state)
{
    std::string _04_5000_chars = lexer_data.at("04-5000-chars");

    for (auto _ : state)
    {
        lexer::Tokens tokens = *lexer::lex(_04_5000_chars);
        benchmark::DoNotOptimize(tokens);
    }
};

BENCHMARK_F(LexerDataFixture, Lex10000Chars)(benchmark::State &state)
{
    std::string _05_10000_chars = lexer_data.at("05-10000-chars");

    for (auto _ : state)
    {
        lexer::Tokens tokens = *lexer::lex(_05_10000_chars);
        benchmark::DoNotOptimize(tokens);
    }
};

BENCHMARK_F(LexerDataFixture, Lex20000Chars)(benchmark::State &state)
{
    std::string _06_20000_chars = lexer_data.at("06-20000-chars");

    for (auto _ : state)
    {
        lexer::Tokens tokens = *lexer::lex(_06_20000_chars);
        benchmark::DoNotOptimize(tokens);
    }
};

BENCHMARK_F(LexerDataFixture, LexBigComment)(benchmark::State &state)
{
    std::string _07_big_comment = lexer_data.at("07-big-comment");

    for (auto _ : state)
    {
        lexer::Tokens tokens = *lexer::lex(_07_big_comment);
        benchmark::DoNotOptimize(tokens);
    }
};

BENCHMARK_MAIN();
