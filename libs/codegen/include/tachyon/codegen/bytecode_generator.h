#pragma once

#include "tachyon/parser/ast.h"
#include "tachyon/runtime/vm.h"

#include <vector>

using namespace tachyon::parser;

namespace tachyon::codegen
{
struct BytecodeGenerator
{
    std::vector<uint8_t> bc;
    std::vector<runtime::Value> constants;
    // TODO: is this the best way to do this? consider how it is finding vars by
    //  name. also, since its unique consider std::set.
    std::vector<std::string> vars;
    // TODO: this certainly does not work because even if we set the value
    //  during construction, it still won't be valid if you declare a new
    //  variable and then use it (will point to newest var). and not the
    //  simplest fix because what if you use it, *then* create the new vars,
    //  then use it again?
    size_t next_free_register = 1;
    uint8_t curr;
    std::vector<Error> errors;

    BytecodeGenerator() {};
    explicit BytecodeGenerator(std::vector<std::string> vars) : vars(std::move(vars))
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
    auto generator = BytecodeGenerator(std::move(args));
    std::visit(generator, e.kind);
    if (!generator.errors.empty())
        return std::unexpected(Error::createMultiple(generator.errors));
    return runtime::Proto(generator.bc, std::move(generator.constants), size, "<anonymous>",
                          e.span);
}
} // namespace tachyon::codegen