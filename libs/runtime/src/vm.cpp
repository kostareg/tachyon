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
        /* machine */
        default:
        case RETV:
        {
            return {};
        }
        case RETC:
        {
            uint8_t src0 = proto.bytecode[++ptr];
            TY_ASSERT(src0 < proto.constants.size());
            call_stack.back().returns = proto.constants[src0];
            return {};
        }
        case RETR:
        {
            uint8_t src0 = proto.bytecode[++ptr];
            call_stack.back().returns = call_stack.back().registers[src0];
            return {};
        }
        case NOOP:
        {
            break;
        }

        /* register */
        case LOCR:
        {
            uint8_t src0 = proto.bytecode[++ptr];
            TY_ASSERT(src0 < proto.constants.size());
            uint8_t dst0 = proto.bytecode[++ptr];
            call_stack.back().registers[dst0] = proto.constants[src0];
            break;
        }
        case LORR:
        {
            uint8_t src0 = proto.bytecode[++ptr];
            uint8_t dst0 = proto.bytecode[++ptr];
            call_stack.back().registers[dst0] = call_stack.back().registers[src0];
            break;
        }

        /* boolean logic */
        case BNOC:
        {
            uint8_t src0 = proto.bytecode[++ptr];
            uint8_t dst0 = proto.bytecode[++ptr];
            call_stack.back().registers[dst0] = !std::get<bool>(proto.constants[src0]);
            break;
        }
        case BNOR:
        {
            uint8_t src0 = proto.bytecode[++ptr];
            uint8_t dst0 = proto.bytecode[++ptr];
            call_stack.back().registers[dst0] = !std::get<bool>(call_stack.back().registers[src0]);
            break;
        }
        case BACC:
        {
            uint8_t src0 = proto.bytecode[++ptr];
            uint8_t src1 = proto.bytecode[++ptr];
            uint8_t dst0 = proto.bytecode[++ptr];
            call_stack.back().registers[dst0] =
                std::get<bool>(proto.constants[src0]) && std::get<bool>(proto.constants[src1]);
            break;
        }
        case BOCC:
        {
            uint8_t src0 = proto.bytecode[++ptr];
            uint8_t src1 = proto.bytecode[++ptr];
            uint8_t dst0 = proto.bytecode[++ptr];
            call_stack.back().registers[dst0] =
                std::get<bool>(proto.constants[src0]) || std::get<bool>(proto.constants[src1]);
            break;
        }
        case BARC:
        {
            uint8_t src0 = proto.bytecode[++ptr];
            uint8_t src1 = proto.bytecode[++ptr];
            uint8_t dst0 = proto.bytecode[++ptr];
            call_stack.back().registers[dst0] = std::get<bool>(call_stack.back().registers[src0]) &&
                                                std::get<bool>(proto.constants[src1]);
            break;
        }
        case BORC:
        {
            uint8_t src0 = proto.bytecode[++ptr];
            uint8_t src1 = proto.bytecode[++ptr];
            uint8_t dst0 = proto.bytecode[++ptr];
            call_stack.back().registers[dst0] = std::get<bool>(call_stack.back().registers[src0]) ||
                                                std::get<bool>(proto.constants[src1]);
            break;
        }
        case BACR:
        {
            uint8_t src0 = proto.bytecode[++ptr];
            uint8_t src1 = proto.bytecode[++ptr];
            uint8_t dst0 = proto.bytecode[++ptr];
            call_stack.back().registers[dst0] = std::get<bool>(proto.constants[src0]) &&
                                                std::get<bool>(call_stack.back().registers[src1]);
            break;
        }
        case BOCR:
        {
            uint8_t src0 = proto.bytecode[++ptr];
            uint8_t src1 = proto.bytecode[++ptr];
            uint8_t dst0 = proto.bytecode[++ptr];
            call_stack.back().registers[dst0] = std::get<bool>(proto.constants[src0]) ||
                                                std::get<bool>(call_stack.back().registers[src1]);
            break;
        }
        case BARR:
        {
            uint8_t src0 = proto.bytecode[++ptr];
            uint8_t src1 = proto.bytecode[++ptr];
            uint8_t dst0 = proto.bytecode[++ptr];
            call_stack.back().registers[dst0] = std::get<bool>(call_stack.back().registers[src0]) &&
                                                std::get<bool>(call_stack.back().registers[src1]);
            break;
        }
        case BORR:
        {
            uint8_t src0 = proto.bytecode[++ptr];
            uint8_t src1 = proto.bytecode[++ptr];
            uint8_t dst0 = proto.bytecode[++ptr];
            call_stack.back().registers[dst0] = std::get<bool>(call_stack.back().registers[src0]) ||
                                                std::get<bool>(call_stack.back().registers[src1]);
            break;
        }

        /* positional */
        // TODO: if you implement integer or size_t type in Values, use that in JMPC/JMPR instead
        case JMPC:
        {
            uint8_t src0 = proto.bytecode[++ptr];
            TY_ASSERT(src0 < proto.constants.size());
            TY_ASSERT(std::holds_alternative<double>(proto.constants[src0]));
            ptr = std::get<double>(proto.constants[src0]);
            continue; // do not increment
        }
        case JMPR:
        {
            uint8_t src0 = proto.bytecode[++ptr];
            TY_ASSERT(std::holds_alternative<double>(call_stack.back().registers[src0]));
            ptr = std::get<double>(call_stack.back().registers[src0]);
            continue;
        }
        case JICC:
        {
            uint8_t src0 = proto.bytecode[++ptr];
            uint8_t dst0 = proto.bytecode[++ptr];
            if (std::get<bool>(proto.constants[src0]))
                ptr = std::get<double>(proto.constants[dst0]);
            continue;
        }
        case JIRC:
        {
            uint8_t src0 = proto.bytecode[++ptr];
            uint8_t dst0 = proto.bytecode[++ptr];
            if (std::get<bool>(call_stack.back().registers[src0]))
                ptr = std::get<double>(proto.constants[dst0]);
            continue;
        }
        case JICR:
        {
            uint8_t src0 = proto.bytecode[++ptr];
            uint8_t dst0 = proto.bytecode[++ptr];
            if (std::get<bool>(proto.constants[src0]))
                ptr = std::get<double>(call_stack.back().registers[dst0]);
            continue;
        }
        case JIRR:
        {
            uint8_t src0 = proto.bytecode[++ptr];
            uint8_t dst0 = proto.bytecode[++ptr];
            if (std::get<bool>(call_stack.back().registers[src0]))
                ptr = std::get<double>(call_stack.back().registers[dst0]);
            continue;
        }

        /* arithmetic */
        // TODO: for now, just assume that math operations are only working on
        //  doubles. we can handle other issues either in the typechecker or by
        //  allowing weak types. or, we can make it a runtime error. Or consider
        //  using TY_ASSERT for all of these.
        case MACC:
        {
            uint8_t src0 = proto.bytecode[++ptr];
            uint8_t src1 = proto.bytecode[++ptr];
            uint8_t dst0 = proto.bytecode[++ptr];
            call_stack.back().registers[dst0] =
                std::get<double>(proto.constants[src0]) + std::get<double>(proto.constants[src1]);
            break;
        }
        case MSCC:
        {
            uint8_t src0 = proto.bytecode[++ptr];
            uint8_t src1 = proto.bytecode[++ptr];
            uint8_t dst0 = proto.bytecode[++ptr];
            call_stack.back().registers[dst0] =
                std::get<double>(proto.constants[src0]) - std::get<double>(proto.constants[src1]);
            break;
        }
        case MMCC:
        {
            uint8_t src0 = proto.bytecode[++ptr];
            uint8_t src1 = proto.bytecode[++ptr];
            uint8_t dst0 = proto.bytecode[++ptr];
            call_stack.back().registers[dst0] =
                std::get<double>(proto.constants[src0]) * std::get<double>(proto.constants[src1]);
            break;
        }
        case MDCC:
        {
            uint8_t src0 = proto.bytecode[++ptr];
            uint8_t src1 = proto.bytecode[++ptr];
            uint8_t dst0 = proto.bytecode[++ptr];
            call_stack.back().registers[dst0] =
                std::get<double>(proto.constants[src0]) / std::get<double>(proto.constants[src1]);
            break;
        }
        case MPCC:
        {
            uint8_t src0 = proto.bytecode[++ptr];
            uint8_t src1 = proto.bytecode[++ptr];
            uint8_t dst0 = proto.bytecode[++ptr];
            call_stack.back().registers[dst0] = pow(std::get<double>(proto.constants[src0]),
                                                    std::get<double>(proto.constants[src1]));
            break;
        }
        case MARC:
        {
            uint8_t src0 = proto.bytecode[++ptr];
            uint8_t src1 = proto.bytecode[++ptr];
            uint8_t dst0 = proto.bytecode[++ptr];
            call_stack.back().registers[dst0] =
                std::get<double>(call_stack.back().registers[src0]) +
                std::get<double>(proto.constants[src1]);
            break;
        }
        case MSRC:
        {
            uint8_t src0 = proto.bytecode[++ptr];
            uint8_t src1 = proto.bytecode[++ptr];
            uint8_t dst0 = proto.bytecode[++ptr];
            call_stack.back().registers[dst0] =
                std::get<double>(call_stack.back().registers[src0]) -
                std::get<double>(proto.constants[src1]);
            break;
        }
        case MMRC:
        {
            uint8_t src0 = proto.bytecode[++ptr];
            uint8_t src1 = proto.bytecode[++ptr];
            uint8_t dst0 = proto.bytecode[++ptr];
            call_stack.back().registers[dst0] =
                std::get<double>(call_stack.back().registers[src0]) *
                std::get<double>(proto.constants[src1]);
            break;
        }
        case MDRC:
        {
            uint8_t src0 = proto.bytecode[++ptr];
            uint8_t src1 = proto.bytecode[++ptr];
            uint8_t dst0 = proto.bytecode[++ptr];
            call_stack.back().registers[dst0] =
                std::get<double>(call_stack.back().registers[src0]) /
                std::get<double>(proto.constants[src1]);
            break;
        }
        case MPRC:
        {
            uint8_t src0 = proto.bytecode[++ptr];
            uint8_t src1 = proto.bytecode[++ptr];
            uint8_t dst0 = proto.bytecode[++ptr];
            call_stack.back().registers[dst0] =
                pow(std::get<double>(call_stack.back().registers[src0]),
                    std::get<double>(proto.constants[src1]));
            break;
        }
        case MACR:
        {
            uint8_t src0 = proto.bytecode[++ptr];
            uint8_t src1 = proto.bytecode[++ptr];
            uint8_t dst0 = proto.bytecode[++ptr];
            call_stack.back().registers[dst0] = std::get<double>(proto.constants[src0]) +
                                                std::get<double>(call_stack.back().registers[src1]);
            break;
        }
        case MSCR:
        {
            uint8_t src0 = proto.bytecode[++ptr];
            uint8_t src1 = proto.bytecode[++ptr];
            uint8_t dst0 = proto.bytecode[++ptr];
            call_stack.back().registers[dst0] = std::get<double>(proto.constants[src0]) -
                                                std::get<double>(call_stack.back().registers[src1]);
            break;
        }
        case MMCR:
        {
            uint8_t src0 = proto.bytecode[++ptr];
            uint8_t src1 = proto.bytecode[++ptr];
            uint8_t dst0 = proto.bytecode[++ptr];
            call_stack.back().registers[dst0] = std::get<double>(proto.constants[src0]) *
                                                std::get<double>(call_stack.back().registers[src1]);
            break;
        }
        case MDCR:
        {
            uint8_t src0 = proto.bytecode[++ptr];
            uint8_t src1 = proto.bytecode[++ptr];
            uint8_t dst0 = proto.bytecode[++ptr];
            call_stack.back().registers[dst0] = std::get<double>(proto.constants[src0]) /
                                                std::get<double>(call_stack.back().registers[src1]);
            break;
        }
        case MPCR:
        {
            uint8_t src0 = proto.bytecode[++ptr];
            uint8_t src1 = proto.bytecode[++ptr];
            uint8_t dst0 = proto.bytecode[++ptr];
            call_stack.back().registers[dst0] =
                pow(std::get<double>(proto.constants[src0]),
                    std::get<double>(call_stack.back().registers[src1]));
            break;
        }
        case MARR:
        {
            uint8_t src0 = proto.bytecode[++ptr];
            uint8_t src1 = proto.bytecode[++ptr];
            uint8_t dst0 = proto.bytecode[++ptr];
            call_stack.back().registers[dst0] =
                std::get<double>(call_stack.back().registers[src0]) +
                std::get<double>(call_stack.back().registers[src1]);
            break;
        }
        case MSRR:
        {
            uint8_t src0 = proto.bytecode[++ptr];
            uint8_t src1 = proto.bytecode[++ptr];
            uint8_t dst0 = proto.bytecode[++ptr];
            call_stack.back().registers[dst0] =
                std::get<double>(call_stack.back().registers[src0]) -
                std::get<double>(call_stack.back().registers[src1]);
            break;
        }
        case MMRR:
        {
            uint8_t src0 = proto.bytecode[++ptr];
            uint8_t src1 = proto.bytecode[++ptr];
            uint8_t dst0 = proto.bytecode[++ptr];
            call_stack.back().registers[dst0] =
                std::get<double>(call_stack.back().registers[src0]) *
                std::get<double>(call_stack.back().registers[src1]);
            break;
        }
        case MDRR:
        {
            uint8_t src0 = proto.bytecode[++ptr];
            uint8_t src1 = proto.bytecode[++ptr];
            uint8_t dst0 = proto.bytecode[++ptr];
            call_stack.back().registers[dst0] =
                std::get<double>(call_stack.back().registers[src0]) /
                std::get<double>(call_stack.back().registers[src1]);
            break;
        }
        case MPRR:
        {
            uint8_t src0 = proto.bytecode[++ptr];
            uint8_t src1 = proto.bytecode[++ptr];
            uint8_t dst0 = proto.bytecode[++ptr];
            call_stack.back().registers[dst0] =
                pow(std::get<double>(proto.constants[src0]),
                    std::get<double>(call_stack.back().registers[src1]));
            break;
        }

        /* comparison */
        // TODO: don't allow function comparison in type checker
        case CECC:
        {
            uint8_t src0 = proto.bytecode[++ptr];
            uint8_t src1 = proto.bytecode[++ptr];
            uint8_t dst0 = proto.bytecode[++ptr];
            call_stack.back().registers[dst0] = proto.constants[src0] == proto.constants[src1];
            break;
        }
        case CNCC:
        {
            uint8_t src0 = proto.bytecode[++ptr];
            uint8_t src1 = proto.bytecode[++ptr];
            uint8_t dst0 = proto.bytecode[++ptr];
            call_stack.back().registers[dst0] = proto.constants[src0] != proto.constants[src1];
            break;
        }
        case CLCC:
        {
            uint8_t src0 = proto.bytecode[++ptr];
            uint8_t src1 = proto.bytecode[++ptr];
            uint8_t dst0 = proto.bytecode[++ptr];
            call_stack.back().registers[dst0] = proto.constants[src0] < proto.constants[src1];
            break;
        }
        case CGCC:
        {
            uint8_t src0 = proto.bytecode[++ptr];
            uint8_t src1 = proto.bytecode[++ptr];
            uint8_t dst0 = proto.bytecode[++ptr];
            call_stack.back().registers[dst0] = proto.constants[src0] > proto.constants[src1];
            break;
        }
        case CHCC:
        {
            uint8_t src0 = proto.bytecode[++ptr];
            uint8_t src1 = proto.bytecode[++ptr];
            uint8_t dst0 = proto.bytecode[++ptr];
            call_stack.back().registers[dst0] = proto.constants[src0] <= proto.constants[src1];
            break;
        }
        case CFCC:
        {
            uint8_t src0 = proto.bytecode[++ptr];
            uint8_t src1 = proto.bytecode[++ptr];
            uint8_t dst0 = proto.bytecode[++ptr];
            call_stack.back().registers[dst0] = proto.constants[src0] >= proto.constants[src1];
            break;
        }
        case CERC:
        {
            uint8_t src0 = proto.bytecode[++ptr];
            uint8_t src1 = proto.bytecode[++ptr];
            uint8_t dst0 = proto.bytecode[++ptr];
            call_stack.back().registers[dst0] =
                call_stack.back().registers[src0] == proto.constants[src1];
            break;
        }
        case CNRC:
        {
            uint8_t src0 = proto.bytecode[++ptr];
            uint8_t src1 = proto.bytecode[++ptr];
            uint8_t dst0 = proto.bytecode[++ptr];
            call_stack.back().registers[dst0] =
                call_stack.back().registers[src0] != proto.constants[src1];
            break;
        }
        case CLRC:
        {
            uint8_t src0 = proto.bytecode[++ptr];
            uint8_t src1 = proto.bytecode[++ptr];
            uint8_t dst0 = proto.bytecode[++ptr];
            call_stack.back().registers[dst0] =
                call_stack.back().registers[src0] < proto.constants[src1];
            break;
        }
        case CGRC:
        {
            uint8_t src0 = proto.bytecode[++ptr];
            uint8_t src1 = proto.bytecode[++ptr];
            uint8_t dst0 = proto.bytecode[++ptr];
            call_stack.back().registers[dst0] =
                call_stack.back().registers[src0] > proto.constants[src1];
            break;
        }
        case CHRC:
        {
            uint8_t src0 = proto.bytecode[++ptr];
            uint8_t src1 = proto.bytecode[++ptr];
            uint8_t dst0 = proto.bytecode[++ptr];
            call_stack.back().registers[dst0] =
                call_stack.back().registers[src0] <= proto.constants[src1];
            break;
        }
        case CFRC:
        {
            uint8_t src0 = proto.bytecode[++ptr];
            uint8_t src1 = proto.bytecode[++ptr];
            uint8_t dst0 = proto.bytecode[++ptr];
            call_stack.back().registers[dst0] =
                call_stack.back().registers[src0] >= proto.constants[src1];
            break;
        }
        case CECR:
        {
            uint8_t src0 = proto.bytecode[++ptr];
            uint8_t src1 = proto.bytecode[++ptr];
            uint8_t dst0 = proto.bytecode[++ptr];
            call_stack.back().registers[dst0] =
                proto.constants[src0] == call_stack.back().registers[src1];
            break;
        }
        case CNCR:
        {
            uint8_t src0 = proto.bytecode[++ptr];
            uint8_t src1 = proto.bytecode[++ptr];
            uint8_t dst0 = proto.bytecode[++ptr];
            call_stack.back().registers[dst0] =
                proto.constants[src0] != call_stack.back().registers[src1];
            break;
        }
        case CLCR:
        {
            uint8_t src0 = proto.bytecode[++ptr];
            uint8_t src1 = proto.bytecode[++ptr];
            uint8_t dst0 = proto.bytecode[++ptr];
            call_stack.back().registers[dst0] =
                proto.constants[src0] < call_stack.back().registers[src1];
            break;
        }
        case CGCR:
        {
            uint8_t src0 = proto.bytecode[++ptr];
            uint8_t src1 = proto.bytecode[++ptr];
            uint8_t dst0 = proto.bytecode[++ptr];
            call_stack.back().registers[dst0] =
                proto.constants[src0] > call_stack.back().registers[src1];
            break;
        }
        case CHCR:
        {
            uint8_t src0 = proto.bytecode[++ptr];
            uint8_t src1 = proto.bytecode[++ptr];
            uint8_t dst0 = proto.bytecode[++ptr];
            call_stack.back().registers[dst0] =
                proto.constants[src0] <= call_stack.back().registers[src1];
            break;
        }
        case CFCR:
        {
            uint8_t src0 = proto.bytecode[++ptr];
            uint8_t src1 = proto.bytecode[++ptr];
            uint8_t dst0 = proto.bytecode[++ptr];
            call_stack.back().registers[dst0] =
                proto.constants[src0] >= call_stack.back().registers[src1];
            break;
        }
        case CERR:
        {
            uint8_t src0 = proto.bytecode[++ptr];
            uint8_t src1 = proto.bytecode[++ptr];
            uint8_t dst0 = proto.bytecode[++ptr];
            call_stack.back().registers[dst0] =
                call_stack.back().registers[src0] == call_stack.back().registers[src1];
            break;
        }
        case CNRR:
        {
            uint8_t src0 = proto.bytecode[++ptr];
            uint8_t src1 = proto.bytecode[++ptr];
            uint8_t dst0 = proto.bytecode[++ptr];
            call_stack.back().registers[dst0] =
                call_stack.back().registers[src0] != call_stack.back().registers[src1];
            break;
        }
        case CLRR:
        {
            uint8_t src0 = proto.bytecode[++ptr];
            uint8_t src1 = proto.bytecode[++ptr];
            uint8_t dst0 = proto.bytecode[++ptr];
            call_stack.back().registers[dst0] =
                call_stack.back().registers[src0] < call_stack.back().registers[src1];
            break;
        }
        case CGRR:
        {
            uint8_t src0 = proto.bytecode[++ptr];
            uint8_t src1 = proto.bytecode[++ptr];
            uint8_t dst0 = proto.bytecode[++ptr];
            call_stack.back().registers[dst0] =
                call_stack.back().registers[src0] > call_stack.back().registers[src1];
            break;
        }
        case CHRR:
        {
            uint8_t src0 = proto.bytecode[++ptr];
            uint8_t src1 = proto.bytecode[++ptr];
            uint8_t dst0 = proto.bytecode[++ptr];
            call_stack.back().registers[dst0] =
                call_stack.back().registers[src0] <= call_stack.back().registers[src1];
            break;
        }
        case CFRR:
        {
            uint8_t src0 = proto.bytecode[++ptr];
            uint8_t src1 = proto.bytecode[++ptr];
            uint8_t dst0 = proto.bytecode[++ptr];
            call_stack.back().registers[dst0] =
                call_stack.back().registers[src0] >= call_stack.back().registers[src1];
            break;
        }

        /* function */
        case CALC:
        {
            uint8_t src0 = proto.bytecode[++ptr];
            TY_ASSERT(src0 < proto.constants.size());
            uint8_t offset = proto.bytecode[++ptr];
            auto fn = std::get<std::shared_ptr<Proto>>(proto.constants[src0]);

            if (auto called_fn = call(fn, offset); !called_fn)
                return std::unexpected(called_fn.error());
            break;
        }
        case CALR:
        {
            uint8_t src0 = proto.bytecode[++ptr];
            uint8_t offset = proto.bytecode[++ptr];
            auto fn = std::get<std::shared_ptr<Proto>>(call_stack.back().registers[src0]);

            if (auto called_fn = call(fn, offset); !called_fn)
                return std::unexpected(called_fn.error());
            break;
        }

        /* intrinsic */
        case PRNC:
        {
            uint8_t src0 = proto.bytecode[++ptr];
            printValue(proto.constants[src0]);
            break;
        }
        case PRNR:
        {
            uint8_t src0 = proto.bytecode[++ptr];
            printValue(call_stack.back().registers[src0]);
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
