#include "run.h"

#include "tachyon/lexer/lexer.h"
#include "tachyon/parser/parser.h"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <print>
#include <string>

// TODO: this is temporary. currently it applies to the whole file but it should really just apply
//  to strings and that's it.
/**
 * @brief unescapes file input, so that escape chars like \n in strings works.
 * @param input content to unescape
 * @return unescaped string
 */
std::string unescape(const std::string &input)
{
    std::string result;
    result.reserve(input.size());

    for (size_t i = 0; i < input.size(); ++i)
    {
        if (input[i] == '\\' && i + 1 < input.size())
        {
            switch (input[++i])
            {
            case 'n':
                result += '\n';
                break;
            case 't':
                result += '\t';
                break;
            case 'r':
                result += '\r';
                break;
            case '\\':
                result += '\\';
                break;
            case '"':
                result += '"';
                break;
            case '\'':
                result += '\'';
                break;
            case '0':
                result += '\0';
                break;
                // Add more cases if needed
            default:
                result += '\\';
                result += input[i];
            }
        }
        else
        {
            result += input[i];
        }
    }

    return result;
}

int run(char *fileName)
{
    // if bad file...
    if (!std::filesystem::exists(fileName) || !std::filesystem::is_regular_file(fileName))
    {
        std::println(std::cerr, "specified file ({}) does not exist or is a directory.", fileName);
        return 1;
    }

    // load file contents
    std::ifstream file(fileName, std::ios::binary);
    std::string file_contents(std::istreambuf_iterator<char>{file}, {});
    file_contents = unescape(file_contents);

    // pipeline
    auto m = tachyon::lexer::lex(file_contents)
                 .and_then(parser::parse)
                 // .and_then(ast::print)
                 .and_then(ast::generateProto)
                 // .and_then(
                 // [](vm::Proto proto) -> std::expected<vm::Proto, Error>
                 // {
                 // std::println("function {}", proto.name);
                 // for (auto bc : proto.bytecode)
                 // {
                 // std::println("{}", bc);
                 // }
                 // return proto;
                 // })
                 .and_then(
                     [](vm::Proto proto) -> std::expected<void, Error>
                     {
                         vm::VM vm;
                         return vm.run(proto);
                     });

    if (!m)
    {
        Error e = m.error();
        e.source = file_contents;
        std::cerr << e << std::flush;
        return 1;
    }

    return 0;
}
