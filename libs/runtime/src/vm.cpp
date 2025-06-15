#include "tachyon/runtime/vm.h"

#include "tachyon/common/assert.h"
#include "tachyon/runtime/bytecode.h"

#include <expected>
#include <iostream>
#include <iterator>

namespace tachyon::runtime
{
std::expected<void, Error> VM::run(const Proto &proto)
{
    size_t ptr = 0;
    while (ptr < proto.bytecode.size())
    {
        uint8_t op = proto.bytecode[ptr];
        // std::println("0x{:02X}", op);

        switch (op)
        {
        default:
        case RETV:
        {
            return {};
        }
        case RETC:
        {
            uint8_t src1 = proto.bytecode[++ptr];
            TY_ASSERT(src1 < proto.constants.size());
            call_stack.back().returns = proto.constants[src1];
            return {};
        }
        case RETR:
        {
            uint8_t src1 = proto.bytecode[++ptr];
            call_stack.back().returns = call_stack.back().registers[src1];
            return {};
        }
        case NOOP:
        {
            break;
        }
        case LOCR:
        {
            uint8_t src1 = proto.bytecode[++ptr];
            TY_ASSERT(src1 < proto.constants.size());
            uint8_t dst = proto.bytecode[++ptr];
            call_stack.back().registers[dst] = proto.constants[src1];
            break;
        }
        case LORR:
        {
            uint8_t src1 = proto.bytecode[++ptr];
            uint8_t dst = proto.bytecode[++ptr];
            call_stack.back().registers[dst] = call_stack.back().registers[src1];
            break;
        }
        case CRLC:
        {
            uint8_t src1 = proto.bytecode[++ptr];
            uint8_t src2 = proto.bytecode[++ptr];
            TY_ASSERT(src2 < proto.constants.size());
            uint8_t dst = proto.bytecode[++ptr];
            call_stack.back().registers[dst] =
                call_stack.back().registers[src1] < proto.constants[src2];
            break;
        }
        case CRGC:
        {
            uint8_t src1 = proto.bytecode[++ptr];
            uint8_t src2 = proto.bytecode[++ptr];
            TY_ASSERT(src2 < proto.constants.size());
            uint8_t dst = proto.bytecode[++ptr];
            call_stack.back().registers[dst] =
                call_stack.back().registers[src1] > proto.constants[src2];
            break;
        }
        case CRLR:
        {
            uint8_t src1 = proto.bytecode[++ptr];
            uint8_t src2 = proto.bytecode[++ptr];
            uint8_t dst = proto.bytecode[++ptr];
            call_stack.back().registers[dst] =
                call_stack.back().registers[src1] < call_stack.back().registers[src2];
            break;
        }
        case CRGR:
        {
            uint8_t src1 = proto.bytecode[++ptr];
            uint8_t src2 = proto.bytecode[++ptr];
            uint8_t dst = proto.bytecode[++ptr];
            call_stack.back().registers[dst] =
                call_stack.back().registers[src1] > call_stack.back().registers[src2];
            break;
        }
        case CREC:
        {
            uint8_t src1 = proto.bytecode[++ptr];
            uint8_t src2 = proto.bytecode[++ptr];
            TY_ASSERT(src2 < proto.constants.size());
            uint8_t dst = proto.bytecode[++ptr];
            call_stack.back().registers[dst] =
                call_stack.back().registers[src1] == proto.constants[src2];
            break;
        }
        case CRER:
        {
            uint8_t src1 = proto.bytecode[++ptr];
            uint8_t src2 = proto.bytecode[++ptr];
            uint8_t dst = proto.bytecode[++ptr];
            call_stack.back().registers[dst] =
                call_stack.back().registers[src1] == call_stack.back().registers[src2];
            break;
        }
        // TODO: if you implement integer type, use that in JMPC/JMPR instead
        case JMPC:
        {
            uint8_t src1 = proto.bytecode[++ptr];
            TY_ASSERT(src1 < proto.constants.size());
            TY_ASSERT(std::holds_alternative<double>(proto.constants[src1]));
            ptr = std::get<double>(proto.constants[src1]);
            continue; // do not increment
        }
        case JMPR:
        {
            uint8_t src1 = proto.bytecode[++ptr];
            TY_ASSERT(std::holds_alternative<double>(call_stack.back().registers[src1]));
            ptr = std::get<double>(call_stack.back().registers[src1]);
            continue;
        }
        // TODO: for now, just assume that math operations are only working on
        //  doubles. we can handle other issues either in the typechecker or by
        //  allowing weak types. or, we can make it a runtime error. Or consider
        //  using TY_ASSERT for all of these.
        case MACC:
        {
            uint8_t src1 = proto.bytecode[++ptr];
            uint8_t src2 = proto.bytecode[++ptr];
            uint8_t dst = proto.bytecode[++ptr];
            call_stack.back().registers[dst] =
                std::get<double>(proto.constants[src1]) + std::get<double>(proto.constants[src2]);
            break;
        }
        case MSCC:
        {
            uint8_t src1 = proto.bytecode[++ptr];
            uint8_t src2 = proto.bytecode[++ptr];
            uint8_t dst = proto.bytecode[++ptr];
            call_stack.back().registers[dst] =
                std::get<double>(proto.constants[src1]) - std::get<double>(proto.constants[src2]);
            break;
        }
        case MMCC:
        {
            uint8_t src1 = proto.bytecode[++ptr];
            uint8_t src2 = proto.bytecode[++ptr];
            uint8_t dst = proto.bytecode[++ptr];
            call_stack.back().registers[dst] =
                std::get<double>(proto.constants[src1]) * std::get<double>(proto.constants[src2]);
            break;
        }
        case MDCC:
        {
            uint8_t src1 = proto.bytecode[++ptr];
            uint8_t src2 = proto.bytecode[++ptr];
            uint8_t dst = proto.bytecode[++ptr];
            call_stack.back().registers[dst] =
                std::get<double>(proto.constants[src1]) / std::get<double>(proto.constants[src2]);
            break;
        }
        case MPCC:
        {
            uint8_t src1 = proto.bytecode[++ptr];
            uint8_t src2 = proto.bytecode[++ptr];
            uint8_t dst = proto.bytecode[++ptr];
            call_stack.back().registers[dst] = pow(std::get<double>(proto.constants[src1]),
                                                   std::get<double>(proto.constants[src2]));
            break;
        }
        case MARC:
        {
            uint8_t src1 = proto.bytecode[++ptr];
            uint8_t src2 = proto.bytecode[++ptr];
            uint8_t dst = proto.bytecode[++ptr];
            call_stack.back().registers[dst] = std::get<double>(call_stack.back().registers[src1]) +
                                               std::get<double>(proto.constants[src2]);
            break;
        }
        case MSRC:
        {
            uint8_t src1 = proto.bytecode[++ptr];
            uint8_t src2 = proto.bytecode[++ptr];
            uint8_t dst = proto.bytecode[++ptr];
            call_stack.back().registers[dst] = std::get<double>(call_stack.back().registers[src1]) -
                                               std::get<double>(proto.constants[src2]);
            break;
        }
        case MMRC:
        {
            uint8_t src1 = proto.bytecode[++ptr];
            uint8_t src2 = proto.bytecode[++ptr];
            uint8_t dst = proto.bytecode[++ptr];
            call_stack.back().registers[dst] = std::get<double>(call_stack.back().registers[src1]) *
                                               std::get<double>(proto.constants[src2]);
            break;
        }
        case MDRC:
        {
            uint8_t src1 = proto.bytecode[++ptr];
            uint8_t src2 = proto.bytecode[++ptr];
            uint8_t dst = proto.bytecode[++ptr];
            call_stack.back().registers[dst] = std::get<double>(call_stack.back().registers[src1]) /
                                               std::get<double>(proto.constants[src2]);
            break;
        }
        case MPRC:
        {
            uint8_t src1 = proto.bytecode[++ptr];
            uint8_t src2 = proto.bytecode[++ptr];
            uint8_t dst = proto.bytecode[++ptr];
            call_stack.back().registers[dst] =
                pow(std::get<double>(call_stack.back().registers[src1]),
                    std::get<double>(proto.constants[src2]));
            break;
        }
        case MACR:
        {
            uint8_t src1 = proto.bytecode[++ptr];
            uint8_t src2 = proto.bytecode[++ptr];
            uint8_t dst = proto.bytecode[++ptr];
            call_stack.back().registers[dst] = std::get<double>(proto.constants[src1]) +
                                               std::get<double>(call_stack.back().registers[src2]);
            break;
        }
        case MSCR:
        {
            uint8_t src1 = proto.bytecode[++ptr];
            uint8_t src2 = proto.bytecode[++ptr];
            uint8_t dst = proto.bytecode[++ptr];
            call_stack.back().registers[dst] = std::get<double>(proto.constants[src1]) -
                                               std::get<double>(call_stack.back().registers[src2]);
            break;
        }
        case MMCR:
        {
            uint8_t src1 = proto.bytecode[++ptr];
            uint8_t src2 = proto.bytecode[++ptr];
            uint8_t dst = proto.bytecode[++ptr];
            call_stack.back().registers[dst] = std::get<double>(proto.constants[src1]) *
                                               std::get<double>(call_stack.back().registers[src2]);
            break;
        }
        case MDCR:
        {
            uint8_t src1 = proto.bytecode[++ptr];
            uint8_t src2 = proto.bytecode[++ptr];
            uint8_t dst = proto.bytecode[++ptr];
            call_stack.back().registers[dst] = std::get<double>(proto.constants[src1]) /
                                               std::get<double>(call_stack.back().registers[src2]);
            break;
        }
        case MPCR:
        {
            uint8_t src1 = proto.bytecode[++ptr];
            uint8_t src2 = proto.bytecode[++ptr];
            uint8_t dst = proto.bytecode[++ptr];
            call_stack.back().registers[dst] =
                pow(std::get<double>(proto.constants[src1]),
                    std::get<double>(call_stack.back().registers[src2]));
            break;
        }
        case MARR:
        {
            uint8_t src1 = proto.bytecode[++ptr];
            uint8_t src2 = proto.bytecode[++ptr];
            uint8_t dst = proto.bytecode[++ptr];
            call_stack.back().registers[dst] = std::get<double>(call_stack.back().registers[src1]) +
                                               std::get<double>(call_stack.back().registers[src2]);
            break;
        }
        case MSRR:
        {
            uint8_t src1 = proto.bytecode[++ptr];
            uint8_t src2 = proto.bytecode[++ptr];
            uint8_t dst = proto.bytecode[++ptr];
            call_stack.back().registers[dst] = std::get<double>(call_stack.back().registers[src1]) -
                                               std::get<double>(call_stack.back().registers[src2]);
            break;
        }
        case MMRR:
        {
            uint8_t src1 = proto.bytecode[++ptr];
            uint8_t src2 = proto.bytecode[++ptr];
            uint8_t dst = proto.bytecode[++ptr];
            call_stack.back().registers[dst] = std::get<double>(call_stack.back().registers[src1]) *
                                               std::get<double>(call_stack.back().registers[src2]);
            break;
        }
        case MDRR:
        {
            uint8_t src1 = proto.bytecode[++ptr];
            uint8_t src2 = proto.bytecode[++ptr];
            uint8_t dst = proto.bytecode[++ptr];
            call_stack.back().registers[dst] = std::get<double>(call_stack.back().registers[src1]) /
                                               std::get<double>(call_stack.back().registers[src2]);
            break;
        }
        case MPRR:
        {
            uint8_t src1 = proto.bytecode[++ptr];
            uint8_t src2 = proto.bytecode[++ptr];
            uint8_t dst = proto.bytecode[++ptr];
            call_stack.back().registers[dst] =
                pow(std::get<double>(proto.constants[src1]),
                    std::get<double>(call_stack.back().registers[src2]));
            break;
        }
        case CALC:
        {
            uint8_t src1 = proto.bytecode[++ptr];
            TY_ASSERT(src1 < proto.constants.size());
            uint8_t offset = proto.bytecode[++ptr];
            auto fn = std::get<std::shared_ptr<Proto>>(proto.constants[src1]);

            if (auto called_fn = call(fn, offset); !called_fn)
                return std::unexpected(called_fn.error());
            break;
        }
        case CALR:
        {
            uint8_t src1 = proto.bytecode[++ptr];
            uint8_t offset = proto.bytecode[++ptr];
            auto fn = std::get<std::shared_ptr<Proto>>(call_stack.back().registers[src1]);

            if (auto called_fn = call(fn, offset); !called_fn)
                return std::unexpected(called_fn.error());
            break;
        }
        case PRNC:
        {
            uint8_t src1 = proto.bytecode[++ptr];
            printValue(proto.constants[src1]);
            break;
        }
        case PRNR:
        {
            uint8_t src1 = proto.bytecode[++ptr];
            printValue(call_stack.back().registers[src1]);
            break;
        }
        }
        ++ptr;
    }

    return std::unexpected(Error::create(ErrorKind::InternalError, SourceSpan(0, 0),
                                         "should not reach end of vm::run without exit"));
}

std::expected<void, Error> VM::call(std::shared_ptr<Proto> fn, uint8_t offset)
{
    // create the next call frame and prepare it with the first few
    // registers of the old ones.
    std::array<Value, 256> registers;
    for (size_t i = 0; i < fn->arguments; ++i)
    {
        // start at +1 + i <- offset + i
        registers[i + 1] = call_stack.back().registers[i + offset];
    }
    CallFrame frame{std::move(registers), 0.0};
    call_stack.push_back(frame);

    if (auto post_fn = run(*fn); !post_fn)
        return std::unexpected(post_fn.error());

    // load the return value of the function to register 0 of this
    // call frame.
    // TODO: consider just throwing this in the returns slot instead
    //  of registers[0], so that the 0th slot can be used for other
    //  things. blocked by bytecode generation.
    call_stack[call_stack.size() - 2].registers[0] = call_stack.back().returns;

    // remove call frame.
    call_stack.pop_back();

    return {};
}

void VM::doctor() const
{
    std::println("vm state:");
    for (const CallFrame &call : call_stack)
    {
        for (const Value &reg : call.registers)
        {
            printValue(reg);
            std::print(" ");
        }
        std::print("\nreturned: ");
        printValue(call.returns);
        std::println();
    }
}
} // namespace tachyon::runtime
