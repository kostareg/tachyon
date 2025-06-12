#include <cstring>
#include <expected>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <ostream>
#include <string>
#include <variant>

#include "../libs/parser/include/tachyon/parser/ast.h"
#include "../libs/runtime/include/tachyon/runtime/vm.h"
#include "error.h"
#include "lexer.h"
#include "parser.h"

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
    auto m = lexer::lex(file_contents)
                 .and_then(parser::parse)
                 // .and_then(ast::print)
                 .and_then(ast::generateProto)
                 // .and_then([](vm::Proto proto) -> std::expected<vm::Proto,
                 // Error> {
                 //     std::println("function {}", proto.name);
                 //     for (auto bc : proto.bytecode) {
                 //         std::println("{}", bc);
                 //     }
                 //     return proto;
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

int testvm()
{
    // --- define myfn --- //
    std::vector<uint8_t> bytecode_myfn = {vm::NOOP, // no-op --------------------------------------
                                          vm::MARR, 0, 1, 0, // add reg0 + reg1 -> reg0
                                          vm::MARC, 0, 1, 0, // add reg0 + 1 -> reg0
                                          vm::PRNC, 2,       // print `the answer is: `
                                          vm::PRNR, 0,       // print reg0
                                          vm::PRNC, 3,       // print `\n`
                                          vm::RETR, 0};
    std::vector<vm::Value> constants_myfn;
    constants_myfn.emplace_back("ZYXWVU");
    constants_myfn.emplace_back(1.0);
    constants_myfn.emplace_back(">>> the answer is: ");
    constants_myfn.emplace_back("\n");
    vm::Proto myfn_proto{std::move(bytecode_myfn), std::move(constants_myfn), 2, "myfn",
                         SourceSpan(0, 0)};

    // --- define main --- //
    // can also CALC 4 directly
    std::vector<uint8_t> bytecode_main = {vm::LOCR, 0,  1,     // load `ABCDEF` to reg 1
                                          vm::LOCR, 1,  3,     // load `123` to register 3
                                          vm::CREC, 3,  2,  0, // comparison
                                          vm::MACR, 2,  3,  0, // math
                                          vm::MSRR, 0,  3,  5, // math
                                          vm::LOCR, 1,  1,     // load `123` to register 1
                                          vm::LOCR, 4,  10,    // load myfn to reg10
                                          vm::CALR, 10,        // call reg10
                                          vm::RETV};
    std::vector<vm::Value> constants_main;
    const char *x = "ABCDEF";
    constants_main.emplace_back(x);
    constants_main.emplace_back(123.0);
    constants_main.emplace_back(123.2);
    constants_main.emplace_back(0.0);
    constants_main.emplace_back(std::make_shared<vm::Proto>(myfn_proto));
    vm::Proto main_proto{std::move(bytecode_main), std::move(constants_main), 0, "main",
                         SourceSpan(0, 0)};

    // vm
    vm::VM vm;
    if (auto ex = vm.run(main_proto); !ex)
    {
        std::println("failed with error: {} inspect error() for more details",
                     ex.error().message_short);
    }

    vm.diagnose();
    std::println("size: {}", sizeof(constants_main[0]));

    return 0;
}

int testgen()
{
    std::string source =
        "x = 13 * 13; print(\">>> this is from the vm\n>>> \"); print(x); return 0;";

    vm::VM vm;
    auto m = lexer::lex(source)
                 .and_then(parser::parse)
                 .and_then(ast::generateProto)
                 // .and_then(
                 //     [](vm::Proto proto) -> std::expected<vm::Proto, Error>
                 //     {
                 // std::println("function {}", proto.name);
                 // for (auto bc : proto.bytecode) {
                 //     std::println("{}", bc);
                 // }
                 // return proto;
                 // })
                 .and_then([&vm](vm::Proto proto) -> std::expected<void, Error>
                           { return vm.run(proto); });

    if (!m)
    {
        Error e = m.error();
        e.source = source;
        std::cerr << e << std::flush;
        return 1;
    }

    return 0;
}

int testerr()
{
    SourceSpan span(3, 3);
    std::string source = "123456789ABC\nDEFG\nHIJKLMNO";
    Error error = Error::create(ErrorKind::BytecodeGenerationError, span, "some message")
                      .withLongMessage("this is a longer version of that message.")
                      .withHint("this is a hint!")
                      .withHint("another hint :)")
                      .withAdditional(Error::create(ErrorKind::InferenceError, SourceSpan(11, 2),
                                                    "some message 2"));
    error.source = source;
    std::cerr << error << std::flush;
    return 0;
}
