#include "tachyon/codegen/bytecode_generator.hpp"

#include "tachyon/common/assert.hpp"
#include "tachyon/parser/print.hpp"
#include "tachyon/runtime/bytecode.hpp"

namespace tachyon::codegen {
using namespace tachyon::parser;

void BytecodeGenerator::operator()(const LiteralExpr &lit) {
    // convert to value, push to constants, reference that.
    runtime::Value val = std::visit([](auto &&v) -> runtime::Value { return v; }, lit.value);
    constants.push_back(val);
    curr = constants.size() - 1;
};

void BytecodeGenerator::operator()(const FnExpr &fn) {
    std::expected<runtime::Proto, Error> maybe_proto;
    if (fn.arguments.empty()) maybe_proto = generate_proto(std::move(*fn.body));
    else {
        std::vector<std::string> arguments;
        arguments.reserve(fn.arguments.size());
        std::ranges::transform(fn.arguments, std::back_inserter(arguments),
                               [](const auto &p) { return p.first; });
        maybe_proto = generate_proto(std::move(*fn.body), arguments);
    }

    // if there was an error, record it. if the function is impure, propagate to self.
    if (!maybe_proto) errors.push_back(maybe_proto.error());
    else if (!maybe_proto->is_pure) is_pure = false;

    constants.emplace_back<std::shared_ptr<runtime::Proto>>(
        std::make_shared<runtime::Proto>(std::move(maybe_proto.value())));

    curr = constants.size() - 1;
};

void BytecodeGenerator::operator()(const UnaryOperatorExpr &unop) {
    // assume we are using the only unary operator, NOT.
    TY_ASSERT(unop.op == Op::Not);

    std::visit(*this, unop.right->kind);
    if (std::holds_alternative<LetRefExpr>(unop.right->kind) ||
        std::holds_alternative<MatrixRefExpr>(unop.right->kind) ||
        std::holds_alternative<FnCallExpr>(unop.right->kind) ||
        std::holds_alternative<UnaryOperatorExpr>(unop.right->kind) ||
        std::holds_alternative<BinaryOperatorExpr>(unop.right->kind)) {
        // curr holds a register address, use BNOR
        bc.push_back(runtime::BNOR);
        bc.push_back(curr);
        bc.push_back(next_free_register);
    } else if (std::holds_alternative<LiteralExpr>(unop.right->kind) ||
               std::holds_alternative<FnExpr>(unop.right->kind)) {
        // curr holds a constant address, use BNOC
        bc.push_back(runtime::BNOC);
        bc.push_back(curr);
        bc.push_back(next_free_register);
    }

    curr = next_free_register++;
}

// TODO: handle fn calls
void BytecodeGenerator::operator()(const BinaryOperatorExpr &binop) {
    uint16_t op = op_to_uint16_t(binop.op);
    if (std::holds_alternative<LiteralExpr>(binop.left->kind)) {
        // constant lhs
        if (std::holds_alternative<LiteralExpr>(binop.right->kind)) {
            // constant <op> constant
            std::visit(*this, binop.left->kind);
            uint16_t lhs = curr;
            std::visit(*this, binop.right->kind);
            uint16_t rhs = curr;
            bc.push_back(op);
            bc.push_back(lhs);
            bc.push_back(rhs);
            bc.push_back(next_free_register); // target
        } else if (std::holds_alternative<LetRefExpr>(binop.right->kind) ||
                   std::holds_alternative<MatrixRefExpr>(binop.right->kind) ||
                   std::holds_alternative<UnaryOperatorExpr>(binop.right->kind) ||
                   std::holds_alternative<BinaryOperatorExpr>(binop.right->kind)) {
            // constant <op> reference
            std::visit(*this, binop.left->kind);
            uint16_t lhs = curr;
            std::visit(*this, binop.right->kind);
            uint16_t rhs = curr;
            bc.push_back(get_bytecode_nth_group(op, 2));
            bc.push_back(lhs);
            bc.push_back(rhs);
            bc.push_back(next_free_register); // target
        } else if (std::holds_alternative<FnCallExpr>(binop.right->kind)) {
            // constant <op> fn_call()
        }
    } else if (std::holds_alternative<LetRefExpr>(binop.left->kind) ||
               std::holds_alternative<MatrixRefExpr>(binop.left->kind) ||
               std::holds_alternative<UnaryOperatorExpr>(binop.left->kind) ||
               std::holds_alternative<BinaryOperatorExpr>(binop.left->kind)) {
        // reference lhs
        if (std::holds_alternative<LiteralExpr>(binop.right->kind)) {
            // reference <op> constant
            std::visit(*this, binop.left->kind);
            uint16_t lhs = curr;
            std::visit(*this, binop.right->kind);
            uint16_t rhs = curr;
            bc.push_back(get_bytecode_nth_group(op, 1));
            bc.push_back(lhs);
            bc.push_back(rhs);
            bc.push_back(next_free_register); // target
        } else if (std::holds_alternative<LetRefExpr>(binop.right->kind) ||
                   std::holds_alternative<MatrixRefExpr>(binop.right->kind) ||
                   std::holds_alternative<UnaryOperatorExpr>(binop.right->kind) ||
                   std::holds_alternative<BinaryOperatorExpr>(binop.right->kind)) {
            // reference <op> reference
            std::visit(*this, binop.left->kind);
            uint16_t lhs = curr;
            std::visit(*this, binop.right->kind);
            uint16_t rhs = curr;
            bc.push_back(get_bytecode_nth_group(op, 3));
            bc.push_back(lhs);
            bc.push_back(rhs);
            bc.push_back(next_free_register); // target
        } else if (std::holds_alternative<FnCallExpr>(binop.right->kind)) {
            // reference <op> fn_call()
        }
    } else if (std::holds_alternative<FnCallExpr>(binop.left->kind)) {
        // fn_call() lhs
        if (std::holds_alternative<LiteralExpr>(binop.right->kind)) {
            // fn_call() <op> constant
        } else if (std::holds_alternative<LetRefExpr>(binop.right->kind) ||
                   std::holds_alternative<MatrixRefExpr>(binop.right->kind) ||
                   std::holds_alternative<UnaryOperatorExpr>(binop.right->kind) ||
                   std::holds_alternative<BinaryOperatorExpr>(binop.right->kind)) {
            // fn_call() <op> reference
        } else if (std::holds_alternative<FnCallExpr>(binop.right->kind)) {
            // fn_call() <op> fn_call()
        }
    }

    curr = next_free_register++;
};

// TODO: does not check for duplicate variables, this should maybe be a step before generation.
void BytecodeGenerator::operator()(const LetExpr &vdecl) {
    std::visit(*this, vdecl.value->kind);

    // find index of register assigned to this let reference. if it does not exist, create one.
    size_t index;
    if (vars.contains(vdecl.name)) index = vars[vdecl.name];
    else {
        index = next_free_register++;
        vars.insert({vdecl.name, index});
    }

    // push instructions to copy value to register
    if (std::holds_alternative<LiteralExpr>(vdecl.value->kind)) {
        // curr is set to the constant address, use LOCR
        bc.push_back(runtime::LOCR);
        bc.push_back(curr);
        bc.push_back(index);
    } else if (std::holds_alternative<FnExpr>(vdecl.value->kind)) {
        // curr is set to the constant address, use LOCR
        bc.push_back(runtime::LOCR);
        bc.push_back(curr);
        bc.push_back(index);

        // rename function
        std::get<std::shared_ptr<runtime::Proto>>(constants.back())->name = vdecl.name;
    } else {
        // curr holds a register address, use LORR
        bc.push_back(runtime::LORR);
        bc.push_back(curr);
        bc.push_back(index);
    }
    curr = index;
};

// TODO: bad failure for undefined variables, there should be a step before
//  generation.
void BytecodeGenerator::operator()(const LetRefExpr &vref) {
    if (auto it = vars.find(vref.name); it != vars.end()) {
        curr = it->second;
    } else {
        curr = 255; // error
    }
};

void BytecodeGenerator::operator()(const MatrixAssignmentExpr &mass) {
    // todo
    std::visit(*this, mass.row->kind);
    uint16_t src0 = curr;

    // TODO: hack: load the constants to a register instead of having a dedicated bytecode
    //  instruction for each combination.
    if (std::holds_alternative<LiteralExpr>(mass.row->kind) ||
        std::holds_alternative<FnExpr>(mass.row->kind)) {
        // we were given a constant, load it into curr.
        size_t dst0 = next_free_register++;
        bc.push_back(runtime::LOCR);
        bc.push_back(curr);
        bc.push_back(dst0);
        src0 = dst0;
    }

    std::visit(*this, mass.col->kind);
    uint16_t src1 = curr;

    if (std::holds_alternative<LiteralExpr>(mass.col->kind) ||
        std::holds_alternative<FnExpr>(mass.col->kind)) {
        // we were given a constant, load it into curr.
        size_t dst0 = next_free_register++;
        bc.push_back(runtime::LOCR);
        bc.push_back(curr);
        bc.push_back(dst0);
        src1 = dst0;
    }

    // since ref is a LetRefExpr, it will always return a register in curr.
    (*this)(mass.ref);
    uint16_t dst0 = curr;

    std::visit(*this, mass.value->kind);

    if (std::holds_alternative<UnaryOperatorExpr>(mass.value->kind) ||
        std::holds_alternative<BinaryOperatorExpr>(mass.value->kind) ||
        std::holds_alternative<LetRefExpr>(mass.value->kind) ||
        std::holds_alternative<MatrixRefExpr>(mass.value->kind)) {
        // we have the register index.
        bc.push_back(runtime::SRRR);
        bc.push_back(src0);
        bc.push_back(src1);
        bc.push_back(curr);
        bc.push_back(dst0);
    } else if (std::holds_alternative<LiteralExpr>(mass.value->kind) ||
               std::holds_alternative<FnExpr>(mass.value->kind)) {
        // we have the constant index so we should load that.
        bc.push_back(runtime::SRRC);
        bc.push_back(src0);
        bc.push_back(src1);
        bc.push_back(curr);
        bc.push_back(dst0);
    } else if (std::holds_alternative<FnCallExpr>(mass.value->kind)) {
        // answer is in register 0.
        bc.push_back(runtime::SRRR);
        bc.push_back(src0);
        bc.push_back(src1);
        bc.push_back(curr);
        bc.push_back(0);
    } else {
        errors.push_back(Error::create(ErrorKind::BytecodeGenerationError, SourceSpan(0, 0),
                                       "could not generate matrix assignment")
                             .withLongMessage("failed to recognize value type."));
    }
}

void BytecodeGenerator::operator()(const MatrixRefExpr &mref) {
    std::visit(*this, mref.row->kind);
    uint16_t src0 = curr;

    // TODO: hack: load the constants to a register instead of having a dedicated bytecode
    //  instruction for each combination.
    if (std::holds_alternative<LiteralExpr>(mref.row->kind) ||
        std::holds_alternative<FnExpr>(mref.row->kind)) {
        // we were given a constant, load it into curr.
        size_t dst0 = next_free_register++;
        bc.push_back(runtime::LOCR);
        bc.push_back(curr);
        bc.push_back(dst0);
        src0 = dst0;
    }

    std::visit(*this, mref.col->kind);
    uint16_t src1 = curr;

    if (std::holds_alternative<LiteralExpr>(mref.col->kind) ||
        std::holds_alternative<FnExpr>(mref.col->kind)) {
        // we were given a constant, load it into curr.
        size_t dst0 = next_free_register++;
        bc.push_back(runtime::LOCR);
        bc.push_back(curr);
        bc.push_back(dst0);
        src1 = dst0;
    }

    // since ref is a LetRefExpr, it will always return a register in curr.
    (*this)(mref.ref);
    size_t dst0 = next_free_register++;
    bc.push_back(runtime::GRRR);
    bc.push_back(src0);
    bc.push_back(src1);
    bc.push_back(curr);
    bc.push_back(dst0);
    curr = dst0;
}

// TODO: CALC? I don't think the ast can generate that.
void BytecodeGenerator::operator()(const FnCallExpr &fnc) {
    // to run a function, we load the arguments in the first [1, n] registers,
    // then call CALC/CALR with a pointer to the prototype.

    // run all of the arguments first, store their currs, then set registers.
    // TODO: reread this - does it make sense to split into two loops? i need to
    //  store start as next_free_register *after* all argument instructions.
    std::vector<uint16_t> currs;
    for (size_t i = 0; i < fnc.args.size(); ++i) {
        std::visit(*this, fnc.args[i].kind);
        currs.push_back(curr);
    }

    size_t start = next_free_register;
    for (size_t i = 0; i < fnc.args.size(); ++i) {
        if (std::holds_alternative<UnaryOperatorExpr>(fnc.args[i].kind) ||
            std::holds_alternative<BinaryOperatorExpr>(fnc.args[i].kind) ||
            std::holds_alternative<LetRefExpr>(fnc.args[i].kind) ||
            std::holds_alternative<MatrixRefExpr>(fnc.args[i].kind)) {
            // we have the register index.
            bc.push_back(runtime::LORR);
            bc.push_back(currs[i]);
            bc.push_back(next_free_register++);
        } else if (std::holds_alternative<LiteralExpr>(fnc.args[i].kind) ||
                   std::holds_alternative<FnExpr>(fnc.args[i].kind)) {
            // we have the constant index so we should load that.
            bc.push_back(runtime::LOCR);
            bc.push_back(currs[i]);
            bc.push_back(next_free_register++);
        } else if (std::holds_alternative<FnCallExpr>(fnc.args[i].kind)) {
            // answer is in register 0.
            bc.push_back(runtime::LORR);
            bc.push_back(0);
            bc.push_back(next_free_register++);
        } else {
            errors.push_back(Error::create(ErrorKind::BytecodeGenerationError, SourceSpan(0, 0),
                                           "could not generate function call")
                                 .withLongMessage("failed to recognize argument type."));
        }
    }

    if (fnc.ref.name == "print") {
        is_pure = false; // i/o is impure
        bc.push_back(runtime::PRNR);
        bc.push_back(start); // just one arg
        return;
    }

    // TODO: do matrix (array) push/pop operations go here?

    (*this)(fnc.ref); // visit ref
    bc.push_back(runtime::CALR);
    bc.push_back(curr);
    bc.push_back(start);

    // writes output to register 0.
    curr = 0;
};

void BytecodeGenerator::operator()(const WhileLoopExpr &wlop) {
    // record the current position, evaluate condition, store register
    size_t condition_start_position = bc.size();
    std::visit(*this, wlop.condition->kind);

    // store the position that we need to fill in later with the address of the last instruction
    // in the body.
    size_t end_reference_position = bc.size() + 2;
    if (std::holds_alternative<LetRefExpr>(wlop.condition->kind) ||
        std::holds_alternative<MatrixRefExpr>(wlop.condition->kind) ||
        std::holds_alternative<FnCallExpr>(wlop.condition->kind) ||
        std::holds_alternative<UnaryOperatorExpr>(wlop.condition->kind) ||
        std::holds_alternative<BinaryOperatorExpr>(wlop.condition->kind)) {
        // curr holds a register address, use JMRN
        bc.push_back(runtime::JMRN);
        bc.push_back(curr);
        bc.push_back(0); // to be filled in
    } else if (std::holds_alternative<LiteralExpr>(wlop.condition->kind) ||
               std::holds_alternative<FnExpr>(wlop.condition->kind)) {
        // curr holds a constant address, use JMCN
        bc.push_back(runtime::JMCN);
        bc.push_back(curr);
        bc.push_back(0); // to be filled in
    }

    // evaluate loop body
    std::visit(*this, wlop.body->kind);

    // now jump back to condition
    bc.push_back(runtime::JMPU);
    bc.push_back(condition_start_position);

    // don't forget to inform the start where the loop ends, so it can jump here when the condition
    // is false. assumes that there is at the very least a return statement after this.
    bc[end_reference_position] = bc.size();
};

void BytecodeGenerator::operator()(const BreakExpr &) {
    // TODO
};

void BytecodeGenerator::operator()(const ContinueExpr &) {
    // TODO
};

void BytecodeGenerator::operator()(const IfExpr &iff) {
    std::visit(*this, iff.condition->kind);

    // store the position that we need to fill in later with the address of the last instruction
    // in the body.
    size_t end_reference_position = bc.size() + 2;
    if (std::holds_alternative<LetRefExpr>(iff.condition->kind) ||
        std::holds_alternative<MatrixRefExpr>(iff.condition->kind) ||
        std::holds_alternative<FnCallExpr>(iff.condition->kind) ||
        std::holds_alternative<UnaryOperatorExpr>(iff.condition->kind) ||
        std::holds_alternative<BinaryOperatorExpr>(iff.condition->kind)) {
        // curr holds a register address, use JMRN
        bc.push_back(runtime::JMRN);
        bc.push_back(curr);
        bc.push_back(0); // to be filled in
    } else if (std::holds_alternative<LiteralExpr>(iff.condition->kind) ||
               std::holds_alternative<FnExpr>(iff.condition->kind)) {
        // curr holds a constant address, use JMCN
        bc.push_back(runtime::JMCN);
        bc.push_back(curr);
        bc.push_back(0); // to be filled in
    }

    // evaluate if body
    std::visit(*this, iff.body->kind);

    // if we are here, the condition is still true. so we should skip all the else-if and else
    // statements.
    bc.push_back(runtime::JMPU);
    bc.push_back(0); // to be filled in
    size_t success_reference_position = bc.size() - 1;

    // don't forget to inform the start where the loop ends, so it can jump here when the condition
    // is false. assumes that there is at the very least a return statement after this.
    bc[end_reference_position] = bc.size();

    // repeat for else-ifs
    std::vector<size_t> success_reference_positions;
    for (size_t i = 0; i < iff.else_if_conditions.size(); ++i) {
        std::visit(*this, iff.else_if_conditions[i].kind);

        // store the position that we need to fill in later with the address of the last instruction
        // in the body.
        size_t end_reference_position = bc.size() + 2;
        if (std::holds_alternative<LetRefExpr>(iff.condition->kind) ||
            std::holds_alternative<MatrixRefExpr>(iff.condition->kind) ||
            std::holds_alternative<FnCallExpr>(iff.condition->kind) ||
            std::holds_alternative<UnaryOperatorExpr>(iff.condition->kind) ||
            std::holds_alternative<BinaryOperatorExpr>(iff.condition->kind)) {
            // curr holds a register address, use JMRN
            bc.push_back(runtime::JMRN);
            bc.push_back(curr);
            bc.push_back(0); // to be filled in
        } else if (std::holds_alternative<LiteralExpr>(iff.condition->kind) ||
                   std::holds_alternative<FnExpr>(iff.condition->kind)) {
            // curr holds a constant address, use JMCN
            bc.push_back(runtime::JMCN);
            bc.push_back(curr);
            bc.push_back(0); // to be filled in
        }

        // evaluate if body
        std::visit(*this, iff.else_if_bodies[i].kind);

        // if we are here, the condition is still true. so we should skip all the else-if and else
        // statements.
        bc.push_back(runtime::JMPU);
        bc.push_back(0); // to be filled in
        success_reference_positions.push_back(bc.size() - 1);

        // don't forget to inform the start where the loop ends, so it can jump here when the
        // condition is false. assumes that there is at the very least a return statement after
        // this.
        bc[end_reference_position] = bc.size();
    }

    // execute final else, if available
    if (iff.else_body != nullptr) {
        std::visit(*this, iff.else_body->kind);
    }

    // don't forget to tell the original if statement where we are ending, in case it is successful
    // and wants to skip all the else-if/elses. again, assumes that there is at the very least a
    // return statement after this.
    bc[success_reference_position] = bc.size();
    for (size_t success_reference_position : success_reference_positions) {
        bc[success_reference_position] = bc.size();
    }

    // TODO: set curr here if you want to evaluate as an expression. do the same for while.
}

// TODO: should this be stripped already?
void BytecodeGenerator::operator()(const ImportExpr &) {};

// retv, retr, retc.
// TODO: should probably check that each function has a return at the end. and
//  eventually default to return void when there's none.
void BytecodeGenerator::operator()(const ReturnExpr &ret) {
    std::visit(*this, ret.returns->kind);
    if (std::holds_alternative<LetRefExpr>(ret.returns->kind) ||
        std::holds_alternative<MatrixRefExpr>(ret.returns->kind) ||
        std::holds_alternative<FnCallExpr>(ret.returns->kind) ||
        std::holds_alternative<UnaryOperatorExpr>(ret.returns->kind) ||
        std::holds_alternative<BinaryOperatorExpr>(ret.returns->kind)) {
        // curr holds a register address, use RETR
        bc.push_back(runtime::RETR);
        bc.push_back(curr);
    } else if (std::holds_alternative<LiteralExpr>(ret.returns->kind) ||
               std::holds_alternative<FnExpr>(ret.returns->kind)) {
        // curr holds a constant address, use RETC
        bc.push_back(runtime::RETC);
        bc.push_back(curr);
    }
};

void BytecodeGenerator::operator()(const MatrixConstructExpr &mc) {
    constants.emplace_back(Matrix(mc.height, mc.list.size() / mc.height, mc.list.size()));
    size_t addr = next_free_register++;
    bc.push_back(runtime::LOCR);
    bc.push_back(constants.size() - 1);
    bc.push_back(addr);
    for (const Expr &expr : mc.list) {
        std::visit(*this, expr.kind);
        if (std::holds_alternative<LetRefExpr>(expr.kind) ||
            std::holds_alternative<MatrixRefExpr>(expr.kind) ||
            std::holds_alternative<FnCallExpr>(expr.kind) ||
            std::holds_alternative<UnaryOperatorExpr>(expr.kind) ||
            std::holds_alternative<BinaryOperatorExpr>(expr.kind)) {
            // curr holds a register address, use LIUR
            bc.push_back(runtime::LIUR);
            bc.push_back(curr);
            bc.push_back(addr);
        } else if (std::holds_alternative<LiteralExpr>(expr.kind) ||
                   std::holds_alternative<FnExpr>(expr.kind)) {
            // curr holds a constant address, use LIUC
            bc.push_back(runtime::LIUC);
            bc.push_back(curr);
            bc.push_back(addr);
        }
    }

    curr = addr;
}

void BytecodeGenerator::operator()(const SequenceExpr &seq) {
    for (const Expr &expr : seq.sequence) {
        std::visit(*this, expr.kind);
    }
};
} // namespace tachyon::codegen
