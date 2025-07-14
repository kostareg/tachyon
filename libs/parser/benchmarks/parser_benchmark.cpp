#include "tachyon/lexer/token.h"
#include "tachyon/parser/parser.h"
#include "tachyon/parser/print.h"

#include <benchmark/benchmark.h>
#include <fstream>
#include <string>

using namespace tachyon;

class ParserDataFixture : public benchmark::Fixture {
  public:
    void SetUp(const ::benchmark::State &) override {
        const std::string included_files[] = {
            "00-blank",      "01-basic",       "02-100-tokens",  "03-200-tokens",
            "04-500-tokens", "05-1000-tokens", "06-2000-tokens",
        };

        for (const std::string &included_file : included_files) {
            const std::string included_file_path =
                "benchmark-data/" + included_file + ".tachyon-tokens";
            std::ifstream file(included_file_path, std::ios::binary | std::ios::ate);
            std::streamsize size = file.tellg();
            if (size == -1) throw std::runtime_error("could not find file: " + included_file);
            file.seekg(0, std::ios::beg);
            std::string content(size, '\0');
            file.read(content.data(), size);

            // parse the string to a list of tokens
            lexer::Tokens tokens;
            std::vector<lexer::Value> constants;
            for (size_t i = 0; i < content.size(); ++i) {
                size_t start = i;

                // continue reading until space or newline
                while (i < content.size() && content[i] != ' ' && content[i] != '\n')
                    ++i;

                std::string substr = content.substr(start, i - start);
                lexer::TokenType tt = lexer::strToTok(substr);

                // continue consuming for some token types
                if (tt == lexer::IDENT || tt == lexer::STRING) {
                    ++i; // space

                    if (content[i] != '"')
                        throw std::runtime_error("didn't find starting \" for ident");
                    ++i; // "

                    size_t ident_start = i;
                    // continue consuming until "
                    while (i < content.size() && content[i] != '"')
                        ++i;

                    if (i == content.size())
                        throw std::runtime_error("didn't find ending \" for ident");

                    constants.emplace_back(content.substr(ident_start, i - ident_start));
                    tokens.emplace_back(tt, std::string_view(&content[ident_start], &content[i]),
                                        constants.size() - 1);

                    ++i; // "
                } else if (tt == lexer::NUMBER) {
                    ++i; // space

                    size_t number_start = i;
                    // continue consuming until space
                    while (i < content.size() && content[i] != ' ')
                        ++i;

                    constants.emplace_back(std::stod(content.substr(number_start, i - start)));
                    tokens.emplace_back(tt, std::string_view(&content[number_start], &content[i]),
                                        constants.size() - 1);
                } else if (tt == lexer::TRUE) {
                    tokens.emplace_back(tt, std::string_view(&content[start], &content[i]));
                } else if (tt == lexer::FALSE) {
                    tokens.emplace_back(tt, std::string_view(&content[start], &content[i]));
                } else {
                    lexer::Token t(tt, std::string_view(&content[start], &content[i]));
                    tokens.push_back(t);
                }
            }

            parser_data[included_file] = std::move(tokens);
            parser_constants[included_file] = std::move(constants);
        }
    }

    void TearDown(const ::benchmark::State &) override { parser_data.clear(); }

  protected:
    std::unordered_map<std::string, lexer::Tokens> parser_data;
    std::unordered_map<std::string, std::vector<lexer::Value>> parser_constants;
};

BENCHMARK_F(ParserDataFixture, ParseBlank)(benchmark::State &state) {
    lexer::Tokens _00_blank = parser_data.at("00-blank");
    std::vector<lexer::Value> _00_blank_cs = parser_constants.at("00-blank");

    for (auto _ : state) {
        parser::Expr expr = *parser::parse(_00_blank, _00_blank_cs);
        benchmark::DoNotOptimize(expr);
    }
};

BENCHMARK_F(ParserDataFixture, ParseBasic)(benchmark::State &state) {
    lexer::Tokens _01_basic = parser_data.at("01-basic");
    std::vector<lexer::Value> _01_basic_cs = parser_constants.at("01-basic");

    for (auto _ : state) {
        parser::Expr expr = *parser::parse(_01_basic, _01_basic_cs);
        benchmark::DoNotOptimize(expr);
    }
};

BENCHMARK_F(ParserDataFixture, Parse100Tokens)(benchmark::State &state) {
    lexer::Tokens _02_100_tokens = parser_data.at("02-100-tokens");
    std::vector<lexer::Value> _02_100_tokens_cs = parser_constants.at("02-100-tokens");

    for (auto _ : state) {
        parser::Expr expr = *parser::parse(_02_100_tokens, _02_100_tokens_cs);
        benchmark::DoNotOptimize(expr);
    }
};

BENCHMARK_F(ParserDataFixture, Parse200Tokens)(benchmark::State &state) {
    lexer::Tokens _03_200_tokens = parser_data.at("03-200-tokens");
    std::vector<lexer::Value> _03_200_tokens_cs = parser_constants.at("03-200-tokens");

    for (auto _ : state) {
        parser::Expr expr = *parser::parse(_03_200_tokens, _03_200_tokens_cs);
        benchmark::DoNotOptimize(expr);
    }
};

BENCHMARK_F(ParserDataFixture, Parse500Tokens)(benchmark::State &state) {
    lexer::Tokens _04_500_tokens = parser_data.at("04-500-tokens");
    std::vector<lexer::Value> _04_500_tokens_cs = parser_constants.at("04-500-tokens");

    for (auto _ : state) {
        parser::Expr expr = *parser::parse(_04_500_tokens, _04_500_tokens_cs);
        benchmark::DoNotOptimize(expr);
    }
};

BENCHMARK_F(ParserDataFixture, Parse1000Tokens)(benchmark::State &state) {
    lexer::Tokens _05_1000_tokens = parser_data.at("05-1000-tokens");
    std::vector<lexer::Value> _05_1000_tokens_cs = parser_constants.at("05-1000-tokens");

    for (auto _ : state) {
        parser::Expr expr = *parser::parse(_05_1000_tokens, _05_1000_tokens_cs);
        benchmark::DoNotOptimize(expr);
    }
};

BENCHMARK_F(ParserDataFixture, Parse2000Tokens)(benchmark::State &state) {
    lexer::Tokens _06_2000_tokens = parser_data.at("06-2000-tokens");
    std::vector<lexer::Value> _06_2000_tokens_cs = parser_constants.at("06-2000-tokens");

    for (auto _ : state) {
        parser::Expr expr = *parser::parse(_06_2000_tokens, _06_2000_tokens_cs);
        benchmark::DoNotOptimize(expr);
    }
};

BENCHMARK_MAIN();