#pragma once

#include "tachyon/parser/ast.h"
#include "tachyon/runtime/vm.h"

#include <unordered_map>
#include <vector>

using namespace tachyon::parser;

namespace tachyon::codegen
{
struct BytecodeGenerator
{
    std::vector<uint8_t> bc = {};
    std::vector<runtime::Value> constants = {};
    // TODO: is this the best way to do this? consider how it is finding vars by
    //  name. also, since its unique consider std::set.
    std::unordered_map<std::string, size_t> vars = {};
    // TODO: not optimal. consider garbage collection or something to keep track
    //  of the variable so you can free registers when they're not needed
    //  anymore? may need a more complex way to store free registers in that
    //  case. can't even search for std::monostate because people may actually
    //  use the unit type as a value.
    size_t next_free_register = 1;
    uint8_t curr = 0;
    std::vector<Error> errors = {};

    BytecodeGenerator() = default;
    explicit BytecodeGenerator(std::unordered_map<std::string, size_t> vars) : vars(std::move(vars))
    {
        next_free_register = vars.size() + 1;
    };

    void operator()(const LiteralExpr &lit);
    void operator()(const FnExpr &fn);
    void operator()(const BinaryOperatorExpr &binop);
    void operator()(const LetExpr &vdecl);
    void operator()(const LetRefExpr &vref);
    void operator()(const FnCallExpr &fnc);
    void operator()(const ImportExpr &);
    void operator()(const ReturnExpr &ret);
    void operator()(const SequenceExpr &seq);
};

inline std::expected<runtime::Proto, Error> generateProto(Expr e)
{
    auto generator = BytecodeGenerator{};
    std::visit(generator, e.kind);
    if (!generator.errors.empty())
        return std::unexpected(Error::createMultiple(generator.errors));
    return runtime::Proto(generator.bc, std::move(generator.constants), 0, "<anonymous>", e.span);
}

// TODO: I wanted to do this with function overloading or defaults but the
//  and_then calls were not working. clean this up.
inline std::expected<runtime::Proto, Error> generateProtoWithArgs(Expr e,
                                                                  std::vector<std::string> args)
{
    auto size = args.size();
    std::unordered_map<std::string, size_t> map;
    for (size_t i = 1; i <= size; ++i)
    {
        map[args[i - 1]] = i;
    }

    auto generator = BytecodeGenerator(std::move(map));
    std::visit(generator, e.kind);
    if (!generator.errors.empty())
        return std::unexpected(Error::createMultiple(generator.errors));
    return runtime::Proto(generator.bc, std::move(generator.constants), size, "<anonymous>",
                          e.span);
}
} // namespace tachyon::codegen