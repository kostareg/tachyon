#include "tachyon/codegen/bytecode_generator.h"
#include "tachyon/common/op.h"
#include "tachyon/parser/ast.h"
#include "tachyon/parser/print.h"

#include <benchmark/benchmark.h>
#include <fstream>
#include <string>

using namespace tachyon;

class CodegenDataFixture : public benchmark::Fixture
{
  public:
    void SetUp(const ::benchmark::State &) override
    {
        const std::string included_files[] = {
            "00-blank",
            "01-basic",
        };

        for (const std::string &included_file : included_files)
        {
            const std::string included_file_path =
                "benchmark-data/" + included_file + ".tachyon-ast";
            std::ifstream file(included_file_path, std::ios::binary | std::ios::ate);
            std::streamsize size = file.tellg();
            if (size == -1)
                throw std::runtime_error("could not find file: " + included_file);
            file.seekg(0, std::ios::beg);
            std::string content(size, '\0');
            file.read(content.data(), size);

            parser::Exprs exprs;

            // parse the string as an abstract syntax tree
            for (size_t i = 0; i < content.size(); ++i)
            {
                size_t start = i;

                // continue reading until space or newline
                while (i < content.size() && content[i] != ' ' && content[i] != '\n')
                    ++i;

                std::string substr = content.substr(start, i - start);
                if (substr == "LiteralExpr")
                {
                    exprs.emplace_back(parse_literal(content, ++i));
                }
                else if (substr == "UnaryOperatorExpr")
                {
                    ++i; // space

                    // parse operator
                    size_t op_start = i;
                    while (i < content.size() && content[i] != ' ' && content[i] != '\n')
                        ++i;

                    parser::Op op = strToOp(content.substr(op_start, i - op_start));

                    ++i; // space

                    // parse rhs kind
                    size_t kind_start = i;
                    while (i < content.size() && content[i] != ' ' && content[i] != '\n')
                        ++i;
                    std::string kind = content.substr(kind_start, i - kind_start);

                    ++i; // space

                    if (kind == "LiteralExpr")
                    {
                        exprs.emplace_back(parser::UnaryOperatorExpr(
                            op, std::make_unique<parser::Expr>(
                                    parser::Expr(parse_literal(content, i)))));
                    }
                    else
                    {
                        throw std::runtime_error("could not read kind of unary operator expr");
                    }
                }
                else if (substr == "BinaryOperatorExpr")
                {
                    ++i; // space

                    // parse operator
                    size_t op_start = i;
                    while (i < content.size() && content[i] != ' ' && content[i] != '\n')
                        ++i;

                    parser::Op op = strToOp(content.substr(op_start, i - op_start));

                    ++i; // space

                    // parse lhs kind
                    size_t kind_start = i;
                    while (i < content.size() && content[i] != ' ' && content[i] != '\n')
                        ++i;
                    std::string kind = content.substr(kind_start, i - kind_start);

                    ++i; // space

                    parser::ExprRef lhs_expr;
                    if (kind == "LiteralExpr")
                    {
                        lhs_expr =
                            std::make_unique<parser::Expr>(parser::Expr(parse_literal(content, i)));
                    }
                    else
                    {
                        throw std::runtime_error("could not read kind of binary operator expr");
                    }

                    ++i; // space

                    // parse rhs kind
                    size_t rhs_kind_start = i;
                    while (i < content.size() && content[i] != ' ' && content[i] != '\n')
                        ++i;
                    std::string rhs_kind = content.substr(rhs_kind_start, i - rhs_kind_start);

                    ++i; // space

                    parser::ExprRef rhs_expr;
                    if (rhs_kind == "LiteralExpr")
                    {
                        rhs_expr =
                            std::make_unique<parser::Expr>(parser::Expr(parse_literal(content, i)));
                    }
                    else
                    {
                        throw std::runtime_error("could not read rhs_kind of let expr binding");
                    }

                    exprs.emplace_back(
                        parser::BinaryOperatorExpr(op, std::move(lhs_expr), std::move(rhs_expr)));
                }
                else if (substr == "LetExpr")
                {
                    ++i; // space

                    // parse name
                    size_t name_start = i;
                    while (i < content.size() && content[i] != ' ' && content[i] != '\n')
                        ++i;
                    std::string name = content.substr(name_start, i - name_start);

                    ++i; // space

                    // parse the kind
                    size_t kind_start = i;
                    while (i < content.size() && content[i] != ' ' && content[i] != '\n')
                        ++i;
                    std::string kind = content.substr(kind_start, i - kind_start);

                    ++i; // space

                    if (kind == "LiteralExpr")
                    {
                        exprs.emplace_back(
                            parser::LetExpr(name, std::make_unique<parser::Expr>(
                                                      parser::Expr(parse_literal(content, i)))));
                    }
                    else
                    {
                        throw std::runtime_error("could not read kind of let expr binding");
                    }
                }
                else if (substr == "LetRefExpr")
                {
                    ++i; // space

                    // parse name
                    size_t name_start = i;
                    while (i < content.size() && content[i] != ' ' && content[i] != '\n')
                        ++i;
                    std::string name = content.substr(name_start, i - name_start);

                    exprs.emplace_back(parser::LetRefExpr(name));
                }
            }

            codegen_data.try_emplace(included_file,
                                     parser::ExprKind{parser::SequenceExpr(std::move(exprs))});
        }

        // TODO: finish this parser
    }

    void TearDown(const ::benchmark::State &) override { codegen_data.clear(); }

  protected:
    std::unordered_map<std::string, parser::Expr> codegen_data;

  private:
    parser::LiteralExpr parse_literal(std::string content, size_t &i)
    {
        // parse the literal value
        size_t indicator_start = i;
        while (i < content.size() && content[i] != ' ' && content[i] != '\n')
            ++i;
        std::string literal_indicator = content.substr(indicator_start, i - indicator_start);

        if (literal_indicator == "unit")
        {
            return {};
        }
        else if (literal_indicator == "double")
        {
            // parse a double
            size_t double_start = ++i;
            while (i < content.size() && content[i] != ' ' && content[i] != '\n')
                ++i;

            std::string double_str = content.substr(double_start, i - double_start);
            double value = std::stod(double_str);
            return parser::LiteralExpr(value);
        }
        else if (literal_indicator == "string")
        {
            // parse a string
            ++i; // "
            size_t string_start = ++i;
            while (i < content.size() && content[i] != '"')
                ++i;

            std::string string = content.substr(string_start, i - string_start);
            return parser::LiteralExpr(string);
        }
        else if (literal_indicator == "bool")
        {
            // parse a bool
            size_t bool_start = ++i;
            while (i < content.size() && content[i] != ' ' && content[i] != '\n')
                ++i;

            std::string bool_str = content.substr(bool_start, i - bool_start);
            if (bool_str == "false")
            {
                return parser::LiteralExpr(false);
            }
            else if (bool_str == "true")
            {
                return parser::LiteralExpr(true);
            }
            else
            {
                throw std::runtime_error("failed to parse boolean");
            }
        }

        return parser::LiteralExpr(-1.);
    }

    parser::Op strToOp(std::string str)
    {
        if (str == "&&")
            return parser::Op::And;
        else if (str == "||")
            return parser::Op::Or;
        else if (str == "!")
            return parser::Op::Not;
        else if (str == "==")
            return parser::Op::Eq;
        else if (str == "!=")
            return parser::Op::Neq;
        else if (str == "<")
            return parser::Op::Lst;
        else if (str == ">")
            return parser::Op::Grt;
        else if (str == "<=")
            return parser::Op::Lset;
        else if (str == ">=")
            return parser::Op::Gret;
        else if (str == "+")
            return parser::Op::Add;
        else if (str == "-")
            return parser::Op::Sub;
        else if (str == "*")
            return parser::Op::Mul;
        else if (str == "/")
            return parser::Op::Div;
        else if (str == "^")
            return parser::Op::Pow;
        else
            throw std::runtime_error("could not understand operator");
    }
};

BENCHMARK_F(CodegenDataFixture, GenerateBlank)(benchmark::State &state)
{
    parser::Expr _00_blank = std::move(codegen_data.at("00-blank"));

    for (auto _ : state)
    {
        runtime::Proto proto = *codegen::generateMainProto(std::move(_00_blank));
        benchmark::DoNotOptimize(proto);
    }
};

BENCHMARK_DEFINE_F(CodegenDataFixture, GenerateBasic)(benchmark::State &state)
{
    parser::Expr _01_basic = std::move(codegen_data.at("01-basic"));

    for (auto _ : state)
    {
        runtime::Proto proto = *codegen::generateMainProto(std::move(_01_basic));
        benchmark::DoNotOptimize(proto);
    }
};

// TODO: this is due to std::move. Can I make Expr copy?
BENCHMARK_REGISTER_F(CodegenDataFixture, GenerateBasic)->Iterations(1);

BENCHMARK_MAIN();