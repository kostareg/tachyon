#include <expected>
#include <print>
#include <ranges>
#include <variant>

#include "tachyon/parser/ast.h"

namespace tachyon::parser
{
void BytecodeGenerator::operator()(const LiteralExpr &lit)
{
    // convert to value, push to constants, reference that.
    runtime::Value val = std::visit([](auto &&val) -> runtime::Value { return val; }, lit.value);
    constants.push_back(val);
    curr = constants.size() - 1;
};

void BytecodeGenerator::operator()(const FnExpr &fn)
{
    // TODO: see generateProto_with_args definition.
    std::expected<runtime::Proto, Error> maybe_proto = runtime::Proto{
        .bytecode = {},
        .constants = {},
        .arguments = 0,
        .name = "",
        .span = fn.body->span,
    };
    if (fn.arguments.size() == 0)
        maybe_proto = generateProto(std::move(*fn.body));
    else
    {
        std::vector<std::string> arguments;
        arguments.reserve(fn.arguments.size());
        std::ranges::transform(fn.arguments, std::back_inserter(arguments),
                               [](const auto &p) { return p.first; });
        maybe_proto = generateProtoWithArgs(std::move(*fn.body), arguments);
    }

    if (!maybe_proto)
        errors.push_back(maybe_proto.error());

    constants.emplace_back<std::shared_ptr<runtime::Proto>>(
        std::make_shared<runtime::Proto>(std::move(maybe_proto.value())));

    curr = constants.size() - 1;
};

// TODO: handle fn calls
void BytecodeGenerator::operator()(const BinaryOperatorExpr &binop)
{
    uint8_t op = op_to_uint8_t(binop.op);
    if (std::holds_alternative<LiteralExpr>(binop.left->kind))
    {
        // constant lhs
        if (std::holds_alternative<LiteralExpr>(binop.right->kind))
        {
            // constant <op> constant
            std::visit(*this, binop.left->kind);
            auto lhs = curr;
            std::visit(*this, binop.right->kind);
            auto rhs = curr;
            bc.push_back(op);
            bc.push_back(lhs);
            bc.push_back(rhs);
            bc.push_back(next_free_register); // target
        }
        else if (std::holds_alternative<LetRefExpr>(binop.right->kind) ||
                 std::holds_alternative<BinaryOperatorExpr>(binop.right->kind))
        {
            // constant <op> reference
            std::visit(*this, binop.left->kind);
            uint8_t lhs = curr;
            std::visit(*this, binop.right->kind);
            uint8_t rhs = curr;
            bc.push_back(op + 8);
            bc.push_back(lhs);
            bc.push_back(rhs);
            bc.push_back(next_free_register); // target
        }
        else if (std::holds_alternative<FnCallExpr>(binop.right->kind))
        {
            // constant <op> fn_call()
        }
    }
    else if (std::holds_alternative<LetRefExpr>(binop.left->kind) ||
             std::holds_alternative<BinaryOperatorExpr>(binop.left->kind))
    {
        // reference lhs
        if (std::holds_alternative<LiteralExpr>(binop.right->kind))
        {
            // reference <op> constant
            std::visit(*this, binop.left->kind);
            auto lhs = curr;
            std::visit(*this, binop.right->kind);
            auto rhs = curr;
            bc.push_back(op + 4);
            bc.push_back(lhs);
            bc.push_back(rhs);
            bc.push_back(next_free_register); // target
        }
        else if (std::holds_alternative<LetRefExpr>(binop.right->kind) ||
                 std::holds_alternative<BinaryOperatorExpr>(binop.right->kind))
        {
            // reference <op> reference
            std::visit(*this, binop.left->kind);
            auto lhs = curr;
            std::visit(*this, binop.right->kind);
            auto rhs = curr;
            bc.push_back(op + 12);
            bc.push_back(lhs);
            bc.push_back(rhs);
            bc.push_back(next_free_register); // target
        }
        else if (std::holds_alternative<FnCallExpr>(binop.right->kind))
        {
            // reference <op> fn_call()
        }
    }
    else if (std::holds_alternative<FnCallExpr>(binop.left->kind))
    {
        // fn_call() lhs
        if (std::holds_alternative<LiteralExpr>(binop.right->kind))
        {
            // fn_call() <op> constant
        }
        else if (std::holds_alternative<LetRefExpr>(binop.right->kind) ||
                 std::holds_alternative<BinaryOperatorExpr>(binop.right->kind))
        {
            // fn_call() <op> reference
        }
        else if (std::holds_alternative<FnCallExpr>(binop.right->kind))
        {
            // fn_call() <op> fn_call()
        }
    }

    curr = next_free_register++;
};

// TODO: does not check for duplicate variables, this should probably be a step
//  before generation.
void BytecodeGenerator::operator()(const LetExpr &vdecl)
{
    vars.push_back(std::move(vdecl.name));
    auto index = vars.size(); // starts at +1 index
    std::visit(*this, vdecl.value->kind);
    if (std::holds_alternative<LiteralExpr>(vdecl.value->kind) ||
        std::holds_alternative<FnExpr>(vdecl.value->kind))
    {
        // curr is set to the constant address, use LOCR
        bc.push_back(runtime::LOCR);
        bc.push_back(curr);
        bc.push_back(index);
    }
    else
    {
        // curr holds a register address, use LORR
        bc.push_back(runtime::LORR);
        bc.push_back(curr);
        bc.push_back(index);
    }
    curr = index;
};

// TODO: bad failure for undefined variables, there should be a step before
//  generation.
void BytecodeGenerator::operator()(const LetRefExpr &vref)
{
    if (auto it = std::find(vars.begin(), vars.end(), vref.name); it != vars.end())
    {
        curr = std::distance(vars.begin(), it) + 1; // starts at +1
    }
    else
    {
        curr = 255; // error
    }
};

// TODO: CALC? I don't think the ast can generate that.
void BytecodeGenerator::operator()(const FnCallExpr &fnc)
{
    // to run a function, we load the arguments in the first [1, n] registers,
    // then call CALC/CALR with a pointer to the prototype.
    auto start = vars.size() + 1; // start after vars
    auto i = start;
    // TODO: do we need two loops here? once to std::visit, once to
    //  bc.push_back.
    for (auto &arg : fnc.args)
    {
        std::visit(*this, arg.kind);
        if (std::holds_alternative<BinaryOperatorExpr>(arg.kind) ||
            std::holds_alternative<LetRefExpr>(arg.kind))
        {
            // we have the register index.
            bc.push_back(runtime::LORR);
            bc.push_back(curr);
            bc.push_back(i);
        }
        else if (std::holds_alternative<LiteralExpr>(arg.kind))
        {
            // we have the constant index so we should load that.
            bc.push_back(runtime::LOCR);
            bc.push_back(curr);
            bc.push_back(i);
        }
        else if (std::holds_alternative<FnCallExpr>(arg.kind))
        {
            // answer is in register 0.
            bc.push_back(runtime::LORR);
            bc.push_back(0);
            bc.push_back(i);
        }
        else if (std::holds_alternative<FnExpr>(arg.kind))
        {
            // we have the constant index.
            bc.push_back(runtime::LOCR);
            bc.push_back(curr);
            bc.push_back(i);
        }
        else
        {
            errors.push_back(Error::create(ErrorKind::BytecodeGenerationError, SourceSpan(0, 0),
                                           "could not generate function call"));
        }
        ++i; // TODO: i dont like this?
    }

    if (fnc.ref.name == "print")
    {
        bc.push_back(runtime::PRNR);
        bc.push_back(start); // just one arg
        return;
    }

    (*this)(fnc.ref);
    bc.push_back(runtime::CALR);
    bc.push_back(curr);
    bc.push_back(start);

    // writes output to register 0.
    curr = 0;
};

// TODO: should this be stripped already?
void BytecodeGenerator::operator()(const ImportExpr &) {};

// retv, retr, retc.
// TODO: should probably check that each function has a return at the end. and
//  eventually default to return void when there's none.
void BytecodeGenerator::operator()(const ReturnExpr &ret)
{
    std::visit(*this, ret.returns->kind);
    if (std::holds_alternative<LetRefExpr>(ret.returns->kind) ||
        std::holds_alternative<FnCallExpr>(ret.returns->kind) ||
        std::holds_alternative<BinaryOperatorExpr>(ret.returns->kind))
    {
        // curr holds a register address, use RETR
        bc.push_back(runtime::RETR);
        bc.push_back(curr);
    }
    else if (std::holds_alternative<LiteralExpr>(ret.returns->kind) ||
             std::holds_alternative<FnExpr>(ret.returns->kind))
    {
        // curr holds a constant address, use RETC
        bc.push_back(runtime::RETC);
        bc.push_back(curr);
    }
};

void BytecodeGenerator::operator()(const SequenceExpr &seq)
{
    for (auto &expr : seq.sequence)
    {
        std::visit(*this, expr.kind);
    }
};

} // namespace tachyon::parser