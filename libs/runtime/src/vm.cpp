#include "tachyon/runtime/vm.hpp"

#include "tachyon/codegen/machine_generator.hpp"
#include "tachyon/common/assert.hpp"
#include "tachyon/common/log.hpp"
#include "tachyon/runtime/bytecode.hpp"

#include <expected>
#include <iostream>
#include <iterator>

// TODO: consider moving from uint16_t to size_t

namespace tachyon::runtime {
std::expected<void, Error> VM::run(Proto &proto) {
    size_t ptr = 0;
    while (ptr < proto.bytecode.size()) {
        TY_TRACE_HEX(proto.bytecode[ptr], "[" << ptr << "]");

        switch (proto.bytecode[ptr]) {
        /* machine */
        default:
        case RETV: {
            return {};
        }
        case RETC: {
            uint16_t src0 = proto.bytecode[++ptr];
            TY_ASSERT(src0 < proto.constants.size());
            call_stack.back().returns = proto.constants[src0];
            return {};
        }
        case RETR: {
            uint16_t src0 = proto.bytecode[++ptr];
            call_stack.back().returns = call_stack.back().registers[src0];
            return {};
        }
        case NOOP: {
            break;
        }

        /* register */
        case LOCR: {
            uint16_t src0 = proto.bytecode[++ptr];
            TY_ASSERT(src0 < proto.constants.size());
            uint16_t dst0 = proto.bytecode[++ptr];
            call_stack.back().registers[dst0] = proto.constants[src0];
            break;
        }
        case LORR: {
            uint16_t src0 = proto.bytecode[++ptr];
            uint16_t dst0 = proto.bytecode[++ptr];
            call_stack.back().registers[dst0] = call_stack.back().registers[src0];
            break;
        }

        /* boolean logic */
        case BNOC: {
            uint16_t src0 = proto.bytecode[++ptr];
            uint16_t dst0 = proto.bytecode[++ptr];
            call_stack.back().registers[dst0] = !std::get<bool>(proto.constants[src0]);
            break;
        }
        case BNOR: {
            uint16_t src0 = proto.bytecode[++ptr];
            uint16_t dst0 = proto.bytecode[++ptr];
            call_stack.back().registers[dst0] = !std::get<bool>(call_stack.back().registers[src0]);
            break;
        }
        case BACC: {
            uint16_t src0 = proto.bytecode[++ptr];
            uint16_t src1 = proto.bytecode[++ptr];
            uint16_t dst0 = proto.bytecode[++ptr];
            call_stack.back().registers[dst0] =
                std::get<bool>(proto.constants[src0]) && std::get<bool>(proto.constants[src1]);
            break;
        }
        case BOCC: {
            uint16_t src0 = proto.bytecode[++ptr];
            uint16_t src1 = proto.bytecode[++ptr];
            uint16_t dst0 = proto.bytecode[++ptr];
            call_stack.back().registers[dst0] =
                std::get<bool>(proto.constants[src0]) || std::get<bool>(proto.constants[src1]);
            break;
        }
        case BARC: {
            uint16_t src0 = proto.bytecode[++ptr];
            uint16_t src1 = proto.bytecode[++ptr];
            uint16_t dst0 = proto.bytecode[++ptr];
            call_stack.back().registers[dst0] = std::get<bool>(call_stack.back().registers[src0]) &&
                                                std::get<bool>(proto.constants[src1]);
            break;
        }
        case BORC: {
            uint16_t src0 = proto.bytecode[++ptr];
            uint16_t src1 = proto.bytecode[++ptr];
            uint16_t dst0 = proto.bytecode[++ptr];
            call_stack.back().registers[dst0] = std::get<bool>(call_stack.back().registers[src0]) ||
                                                std::get<bool>(proto.constants[src1]);
            break;
        }
        case BACR: {
            uint16_t src0 = proto.bytecode[++ptr];
            uint16_t src1 = proto.bytecode[++ptr];
            uint16_t dst0 = proto.bytecode[++ptr];
            call_stack.back().registers[dst0] = std::get<bool>(proto.constants[src0]) &&
                                                std::get<bool>(call_stack.back().registers[src1]);
            break;
        }
        case BOCR: {
            uint16_t src0 = proto.bytecode[++ptr];
            uint16_t src1 = proto.bytecode[++ptr];
            uint16_t dst0 = proto.bytecode[++ptr];
            call_stack.back().registers[dst0] = std::get<bool>(proto.constants[src0]) ||
                                                std::get<bool>(call_stack.back().registers[src1]);
            break;
        }
        case BARR: {
            uint16_t src0 = proto.bytecode[++ptr];
            uint16_t src1 = proto.bytecode[++ptr];
            uint16_t dst0 = proto.bytecode[++ptr];
            call_stack.back().registers[dst0] = std::get<bool>(call_stack.back().registers[src0]) &&
                                                std::get<bool>(call_stack.back().registers[src1]);
            break;
        }
        case BORR: {
            uint16_t src0 = proto.bytecode[++ptr];
            uint16_t src1 = proto.bytecode[++ptr];
            uint16_t dst0 = proto.bytecode[++ptr];
            call_stack.back().registers[dst0] = std::get<bool>(call_stack.back().registers[src0]) ||
                                                std::get<bool>(call_stack.back().registers[src1]);
            break;
        }

        /* positional */
        case JMPU: {
            ptr = proto.bytecode[++ptr];
            continue; // do not increment
        }
        case JMCI: {
            uint16_t src0 = proto.bytecode[++ptr];
            uint16_t dst0 = proto.bytecode[++ptr];
            if (std::get<bool>(proto.constants[src0])) {
                ptr = dst0;
                continue;
            }
            break;
        }
        case JMCN: {
            uint16_t src0 = proto.bytecode[++ptr];
            uint16_t dst0 = proto.bytecode[++ptr];
            if (!std::get<bool>(proto.constants[src0])) {
                ptr = dst0;
                continue;
            }
            break;
        }
        case JMRI: {
            uint16_t src0 = proto.bytecode[++ptr];
            uint16_t dst0 = proto.bytecode[++ptr];
            if (std::get<bool>(call_stack.back().registers[src0])) {
                ptr = dst0;
                continue;
            }
            break;
        }
        case JMRN: {
            uint16_t src0 = proto.bytecode[++ptr];
            uint16_t dst0 = proto.bytecode[++ptr];
            if (!std::get<bool>(call_stack.back().registers[src0])) {
                ptr = dst0;
                continue;
            }
            break;
        }

        /* arithmetic */
        // TODO: for now, just assume that math operations are only working on
        //  doubles. we can handle other issues either in the typechecker or by
        //  allowing weak types. or, we can make it a runtime error. Or consider
        //  using TY_ASSERT for all of these.
        case MACC: {
            uint16_t src0 = proto.bytecode[++ptr];
            uint16_t src1 = proto.bytecode[++ptr];
            uint16_t dst0 = proto.bytecode[++ptr];
            call_stack.back().registers[dst0] =
                std::get<double>(proto.constants[src0]) + std::get<double>(proto.constants[src1]);
            break;
        }
        case MSCC: {
            uint16_t src0 = proto.bytecode[++ptr];
            uint16_t src1 = proto.bytecode[++ptr];
            uint16_t dst0 = proto.bytecode[++ptr];
            call_stack.back().registers[dst0] =
                std::get<double>(proto.constants[src0]) - std::get<double>(proto.constants[src1]);
            break;
        }
        case MMCC: {
            uint16_t src0 = proto.bytecode[++ptr];
            uint16_t src1 = proto.bytecode[++ptr];
            uint16_t dst0 = proto.bytecode[++ptr];
            call_stack.back().registers[dst0] =
                std::get<double>(proto.constants[src0]) * std::get<double>(proto.constants[src1]);
            break;
        }
        case MDCC: {
            uint16_t src0 = proto.bytecode[++ptr];
            uint16_t src1 = proto.bytecode[++ptr];
            uint16_t dst0 = proto.bytecode[++ptr];
            call_stack.back().registers[dst0] =
                std::get<double>(proto.constants[src0]) / std::get<double>(proto.constants[src1]);
            break;
        }
        case MPCC: {
            uint16_t src0 = proto.bytecode[++ptr];
            uint16_t src1 = proto.bytecode[++ptr];
            uint16_t dst0 = proto.bytecode[++ptr];
            call_stack.back().registers[dst0] = pow(std::get<double>(proto.constants[src0]),
                                                    std::get<double>(proto.constants[src1]));
            break;
        }
        case MARC: {
            uint16_t src0 = proto.bytecode[++ptr];
            uint16_t src1 = proto.bytecode[++ptr];
            uint16_t dst0 = proto.bytecode[++ptr];
            call_stack.back().registers[dst0] =
                std::get<double>(call_stack.back().registers[src0]) +
                std::get<double>(proto.constants[src1]);
            break;
        }
        case MSRC: {
            uint16_t src0 = proto.bytecode[++ptr];
            uint16_t src1 = proto.bytecode[++ptr];
            uint16_t dst0 = proto.bytecode[++ptr];
            call_stack.back().registers[dst0] =
                std::get<double>(call_stack.back().registers[src0]) -
                std::get<double>(proto.constants[src1]);
            break;
        }
        case MMRC: {
            uint16_t src0 = proto.bytecode[++ptr];
            uint16_t src1 = proto.bytecode[++ptr];
            uint16_t dst0 = proto.bytecode[++ptr];
            call_stack.back().registers[dst0] =
                std::get<double>(call_stack.back().registers[src0]) *
                std::get<double>(proto.constants[src1]);
            break;
        }
        case MDRC: {
            uint16_t src0 = proto.bytecode[++ptr];
            uint16_t src1 = proto.bytecode[++ptr];
            uint16_t dst0 = proto.bytecode[++ptr];
            call_stack.back().registers[dst0] =
                std::get<double>(call_stack.back().registers[src0]) /
                std::get<double>(proto.constants[src1]);
            break;
        }
        case MPRC: {
            uint16_t src0 = proto.bytecode[++ptr];
            uint16_t src1 = proto.bytecode[++ptr];
            uint16_t dst0 = proto.bytecode[++ptr];
            call_stack.back().registers[dst0] =
                pow(std::get<double>(call_stack.back().registers[src0]),
                    std::get<double>(proto.constants[src1]));
            break;
        }
        case MACR: {
            uint16_t src0 = proto.bytecode[++ptr];
            uint16_t src1 = proto.bytecode[++ptr];
            uint16_t dst0 = proto.bytecode[++ptr];
            call_stack.back().registers[dst0] = std::get<double>(proto.constants[src0]) +
                                                std::get<double>(call_stack.back().registers[src1]);
            break;
        }
        case MSCR: {
            uint16_t src0 = proto.bytecode[++ptr];
            uint16_t src1 = proto.bytecode[++ptr];
            uint16_t dst0 = proto.bytecode[++ptr];
            call_stack.back().registers[dst0] = std::get<double>(proto.constants[src0]) -
                                                std::get<double>(call_stack.back().registers[src1]);
            break;
        }
        case MMCR: {
            uint16_t src0 = proto.bytecode[++ptr];
            uint16_t src1 = proto.bytecode[++ptr];
            uint16_t dst0 = proto.bytecode[++ptr];
            call_stack.back().registers[dst0] = std::get<double>(proto.constants[src0]) *
                                                std::get<double>(call_stack.back().registers[src1]);
            break;
        }
        case MDCR: {
            uint16_t src0 = proto.bytecode[++ptr];
            uint16_t src1 = proto.bytecode[++ptr];
            uint16_t dst0 = proto.bytecode[++ptr];
            call_stack.back().registers[dst0] = std::get<double>(proto.constants[src0]) /
                                                std::get<double>(call_stack.back().registers[src1]);
            break;
        }
        case MPCR: {
            uint16_t src0 = proto.bytecode[++ptr];
            uint16_t src1 = proto.bytecode[++ptr];
            uint16_t dst0 = proto.bytecode[++ptr];
            call_stack.back().registers[dst0] =
                pow(std::get<double>(proto.constants[src0]),
                    std::get<double>(call_stack.back().registers[src1]));
            break;
        }
        case MARR: {
            uint16_t src0 = proto.bytecode[++ptr];
            uint16_t src1 = proto.bytecode[++ptr];
            uint16_t dst0 = proto.bytecode[++ptr];
            call_stack.back().registers[dst0] =
                std::get<double>(call_stack.back().registers[src0]) +
                std::get<double>(call_stack.back().registers[src1]);
            break;
        }
        case MSRR: {
            uint16_t src0 = proto.bytecode[++ptr];
            uint16_t src1 = proto.bytecode[++ptr];
            uint16_t dst0 = proto.bytecode[++ptr];
            call_stack.back().registers[dst0] =
                std::get<double>(call_stack.back().registers[src0]) -
                std::get<double>(call_stack.back().registers[src1]);
            break;
        }
        case MMRR: {
            uint16_t src0 = proto.bytecode[++ptr];
            uint16_t src1 = proto.bytecode[++ptr];
            uint16_t dst0 = proto.bytecode[++ptr];
            call_stack.back().registers[dst0] =
                std::get<double>(call_stack.back().registers[src0]) *
                std::get<double>(call_stack.back().registers[src1]);
            break;
        }
        case MDRR: {
            uint16_t src0 = proto.bytecode[++ptr];
            uint16_t src1 = proto.bytecode[++ptr];
            uint16_t dst0 = proto.bytecode[++ptr];
            call_stack.back().registers[dst0] =
                std::get<double>(call_stack.back().registers[src0]) /
                std::get<double>(call_stack.back().registers[src1]);
            break;
        }
        case MPRR: {
            uint16_t src0 = proto.bytecode[++ptr];
            uint16_t src1 = proto.bytecode[++ptr];
            uint16_t dst0 = proto.bytecode[++ptr];
            call_stack.back().registers[dst0] =
                pow(std::get<double>(proto.constants[src0]),
                    std::get<double>(call_stack.back().registers[src1]));
            break;
        }

        /* comparison */
        // TODO: support things other than double
        // TODO: don't allow function comparison in type checker
        case CECC: {
            uint16_t src0 = proto.bytecode[++ptr];
            uint16_t src1 = proto.bytecode[++ptr];
            uint16_t dst0 = proto.bytecode[++ptr];
            call_stack.back().registers[dst0] =
                std::get<double>(proto.constants[src0]) == std::get<double>(proto.constants[src1]);
            break;
        }
        case CNCC: {
            uint16_t src0 = proto.bytecode[++ptr];
            uint16_t src1 = proto.bytecode[++ptr];
            uint16_t dst0 = proto.bytecode[++ptr];
            call_stack.back().registers[dst0] =
                std::get<double>(proto.constants[src0]) != std::get<double>(proto.constants[src1]);
            break;
        }
        case CLCC: {
            uint16_t src0 = proto.bytecode[++ptr];
            uint16_t src1 = proto.bytecode[++ptr];
            uint16_t dst0 = proto.bytecode[++ptr];
            call_stack.back().registers[dst0] =
                std::get<double>(proto.constants[src0]) < std::get<double>(proto.constants[src1]);
            break;
        }
        case CGCC: {
            uint16_t src0 = proto.bytecode[++ptr];
            uint16_t src1 = proto.bytecode[++ptr];
            uint16_t dst0 = proto.bytecode[++ptr];
            call_stack.back().registers[dst0] =
                std::get<double>(proto.constants[src0]) > std::get<double>(proto.constants[src1]);
            break;
        }
        case CHCC: {
            uint16_t src0 = proto.bytecode[++ptr];
            uint16_t src1 = proto.bytecode[++ptr];
            uint16_t dst0 = proto.bytecode[++ptr];
            call_stack.back().registers[dst0] =
                std::get<double>(proto.constants[src0]) <= std::get<double>(proto.constants[src1]);
            break;
        }
        case CFCC: {
            uint16_t src0 = proto.bytecode[++ptr];
            uint16_t src1 = proto.bytecode[++ptr];
            uint16_t dst0 = proto.bytecode[++ptr];
            call_stack.back().registers[dst0] =
                std::get<double>(proto.constants[src0]) >= std::get<double>(proto.constants[src1]);
            break;
        }
        case CERC: {
            uint16_t src0 = proto.bytecode[++ptr];
            uint16_t src1 = proto.bytecode[++ptr];
            uint16_t dst0 = proto.bytecode[++ptr];
            call_stack.back().registers[dst0] =
                std::get<double>(call_stack.back().registers[src0]) ==
                std::get<double>(proto.constants[src1]);
            break;
        }
        case CNRC: {
            uint16_t src0 = proto.bytecode[++ptr];
            uint16_t src1 = proto.bytecode[++ptr];
            uint16_t dst0 = proto.bytecode[++ptr];
            call_stack.back().registers[dst0] =
                std::get<double>(call_stack.back().registers[src0]) !=
                std::get<double>(proto.constants[src1]);
            break;
        }
        case CLRC: {
            uint16_t src0 = proto.bytecode[++ptr];
            uint16_t src1 = proto.bytecode[++ptr];
            uint16_t dst0 = proto.bytecode[++ptr];
            call_stack.back().registers[dst0] =
                std::get<double>(call_stack.back().registers[src0]) <
                std::get<double>(proto.constants[src1]);
            break;
        }
        case CGRC: {
            uint16_t src0 = proto.bytecode[++ptr];
            uint16_t src1 = proto.bytecode[++ptr];
            uint16_t dst0 = proto.bytecode[++ptr];
            call_stack.back().registers[dst0] =
                std::get<double>(call_stack.back().registers[src0]) >
                std::get<double>(proto.constants[src1]);
            break;
        }
        case CHRC: {
            uint16_t src0 = proto.bytecode[++ptr];
            uint16_t src1 = proto.bytecode[++ptr];
            uint16_t dst0 = proto.bytecode[++ptr];
            call_stack.back().registers[dst0] =
                std::get<double>(call_stack.back().registers[src0]) <=
                std::get<double>(proto.constants[src1]);
            break;
        }
        case CFRC: {
            uint16_t src0 = proto.bytecode[++ptr];
            uint16_t src1 = proto.bytecode[++ptr];
            uint16_t dst0 = proto.bytecode[++ptr];
            call_stack.back().registers[dst0] =
                std::get<double>(call_stack.back().registers[src0]) >=
                std::get<double>(proto.constants[src1]);
            break;
        }
        case CECR: {
            uint16_t src0 = proto.bytecode[++ptr];
            uint16_t src1 = proto.bytecode[++ptr];
            uint16_t dst0 = proto.bytecode[++ptr];
            call_stack.back().registers[dst0] = std::get<double>(proto.constants[src0]) ==
                                                std::get<double>(call_stack.back().registers[src1]);
            break;
        }
        case CNCR: {
            uint16_t src0 = proto.bytecode[++ptr];
            uint16_t src1 = proto.bytecode[++ptr];
            uint16_t dst0 = proto.bytecode[++ptr];
            call_stack.back().registers[dst0] = std::get<double>(proto.constants[src0]) !=
                                                std::get<double>(call_stack.back().registers[src1]);
            break;
        }
        case CLCR: {
            uint16_t src0 = proto.bytecode[++ptr];
            uint16_t src1 = proto.bytecode[++ptr];
            uint16_t dst0 = proto.bytecode[++ptr];
            call_stack.back().registers[dst0] = std::get<double>(proto.constants[src0]) <
                                                std::get<double>(call_stack.back().registers[src1]);
            break;
        }
        case CGCR: {
            uint16_t src0 = proto.bytecode[++ptr];
            uint16_t src1 = proto.bytecode[++ptr];
            uint16_t dst0 = proto.bytecode[++ptr];
            call_stack.back().registers[dst0] = std::get<double>(proto.constants[src0]) >
                                                std::get<double>(call_stack.back().registers[src1]);
            break;
        }
        case CHCR: {
            uint16_t src0 = proto.bytecode[++ptr];
            uint16_t src1 = proto.bytecode[++ptr];
            uint16_t dst0 = proto.bytecode[++ptr];
            call_stack.back().registers[dst0] = std::get<double>(proto.constants[src0]) <=
                                                std::get<double>(call_stack.back().registers[src1]);
            break;
        }
        case CFCR: {
            uint16_t src0 = proto.bytecode[++ptr];
            uint16_t src1 = proto.bytecode[++ptr];
            uint16_t dst0 = proto.bytecode[++ptr];
            call_stack.back().registers[dst0] = std::get<double>(proto.constants[src0]) >=
                                                std::get<double>(call_stack.back().registers[src1]);
            break;
        }
        case CERR: {
            uint16_t src0 = proto.bytecode[++ptr];
            uint16_t src1 = proto.bytecode[++ptr];
            uint16_t dst0 = proto.bytecode[++ptr];
            call_stack.back().registers[dst0] =
                std::get<double>(call_stack.back().registers[src0]) ==
                std::get<double>(call_stack.back().registers[src1]);
            break;
        }
        case CNRR: {
            uint16_t src0 = proto.bytecode[++ptr];
            uint16_t src1 = proto.bytecode[++ptr];
            uint16_t dst0 = proto.bytecode[++ptr];
            call_stack.back().registers[dst0] =
                std::get<double>(call_stack.back().registers[src0]) !=
                std::get<double>(call_stack.back().registers[src1]);
            break;
        }
        case CLRR: {
            uint16_t src0 = proto.bytecode[++ptr];
            uint16_t src1 = proto.bytecode[++ptr];
            uint16_t dst0 = proto.bytecode[++ptr];
            call_stack.back().registers[dst0] =
                std::get<double>(call_stack.back().registers[src0]) <
                std::get<double>(call_stack.back().registers[src1]);
            break;
        }
        case CGRR: {
            uint16_t src0 = proto.bytecode[++ptr];
            uint16_t src1 = proto.bytecode[++ptr];
            uint16_t dst0 = proto.bytecode[++ptr];
            call_stack.back().registers[dst0] =
                std::get<double>(call_stack.back().registers[src0]) >
                std::get<double>(call_stack.back().registers[src1]);
            break;
        }
        case CHRR: {
            uint16_t src0 = proto.bytecode[++ptr];
            uint16_t src1 = proto.bytecode[++ptr];
            uint16_t dst0 = proto.bytecode[++ptr];
            call_stack.back().registers[dst0] =
                std::get<double>(call_stack.back().registers[src0]) <=
                std::get<double>(call_stack.back().registers[src1]);
            break;
        }
        case CFRR: {
            uint16_t src0 = proto.bytecode[++ptr];
            uint16_t src1 = proto.bytecode[++ptr];
            uint16_t dst0 = proto.bytecode[++ptr];
            call_stack.back().registers[dst0] =
                std::get<double>(call_stack.back().registers[src0]) >=
                std::get<double>(call_stack.back().registers[src1]);
            break;
        }

        /* lists */
        case LIUC: {
            uint16_t src0 = proto.bytecode[++ptr];
            uint16_t dst0 = proto.bytecode[++ptr];
            std::get<Matrix>(call_stack.back().registers[dst0])
                .push_back(std::get<double>(proto.constants[src0]));
            break;
        }
        case LIUR: {
            uint16_t src0 = proto.bytecode[++ptr];
            uint16_t dst0 = proto.bytecode[++ptr];
            std::get<Matrix>(call_stack.back().registers[dst0])
                .push_back(std::get<double>(call_stack.back().registers[src0]));
            break;
        }
        case LIOR: {
            uint16_t src0 = proto.bytecode[++ptr];
            uint16_t dst0 = proto.bytecode[++ptr];
            call_stack.back().registers[dst0] =
                std::get<Matrix>(call_stack.back().registers[src0]).pop_back();
            break;
        }
        // TODO: these use narrowing conversions. have a dedicated size type or come up with another
        //  solution.
        case GRRC: {
            uint16_t src0 = proto.bytecode[++ptr];
            uint16_t src1 = proto.bytecode[++ptr];
            uint16_t src2 = proto.bytecode[++ptr];
            uint16_t dst0 = proto.bytecode[++ptr];
            call_stack.back().registers[dst0] = std::get<Matrix>(proto.constants[src2])(
                static_cast<size_t>(
                    std::round(std::get<double>(call_stack.back().registers[src0]))),
                static_cast<size_t>(
                    std::round(std::get<double>(call_stack.back().registers[src1]))));
            break;
        }
        case GRRR: {
            uint16_t src0 = proto.bytecode[++ptr];
            uint16_t src1 = proto.bytecode[++ptr];
            uint16_t src2 = proto.bytecode[++ptr];
            uint16_t dst0 = proto.bytecode[++ptr];
            call_stack.back().registers[dst0] = std::get<Matrix>(call_stack.back().registers[src2])(
                static_cast<size_t>(
                    std::round(std::get<double>(call_stack.back().registers[src0]))),
                static_cast<size_t>(
                    std::round(std::get<double>(call_stack.back().registers[src1]))));
            break;
        }
        case SRRC: {
            uint16_t src0 = proto.bytecode[++ptr];
            uint16_t src1 = proto.bytecode[++ptr];
            uint16_t src2 = proto.bytecode[++ptr];
            uint16_t dst0 = proto.bytecode[++ptr];
            std::get<Matrix>(call_stack.back().registers[dst0])(
                static_cast<size_t>(
                    std::round(std::get<double>(call_stack.back().registers[src0]))),
                static_cast<size_t>(
                    std::round(std::get<double>(call_stack.back().registers[src1])))) =
                std::get<double>(proto.constants[src2]);
            break;
        }
        case SRRR: {
            uint16_t src0 = proto.bytecode[++ptr];
            uint16_t src1 = proto.bytecode[++ptr];
            uint16_t src2 = proto.bytecode[++ptr];
            uint16_t dst0 = proto.bytecode[++ptr];
            std::get<Matrix>(call_stack.back().registers[dst0])(
                static_cast<size_t>(
                    std::round(std::get<double>(call_stack.back().registers[src0]))),
                static_cast<size_t>(
                    std::round(std::get<double>(call_stack.back().registers[src1])))) =
                std::get<double>(call_stack.back().registers[src2]);
            break;
        }

        /* function */
        case CALC: {
            uint16_t src0 = proto.bytecode[++ptr];
            TY_ASSERT(src0 < proto.constants.size());
            uint16_t offset = proto.bytecode[++ptr];
            auto fn = std::get<std::shared_ptr<Proto>>(proto.constants[src0]);

            if (auto called_fn = call(fn, offset); !called_fn)
                return std::unexpected(called_fn.error());
            break;
        }
        case CALR: {
            uint16_t src0 = proto.bytecode[++ptr];
            uint16_t offset = proto.bytecode[++ptr];
            auto fn = std::get<std::shared_ptr<Proto>>(call_stack.back().registers[src0]);

            if (auto called_fn = call(fn, offset); !called_fn)
                return std::unexpected(called_fn.error());
            break;
        }

        /* intrinsic */
        case PRNC: {
            uint16_t src0 = proto.bytecode[++ptr];
            print_value(proto.constants[src0]);
            break;
        }
        case PRNR: {
            uint16_t src0 = proto.bytecode[++ptr];
            print_value(call_stack.back().registers[src0]);
            break;
        }
        }
        ++ptr;
    }

    switch (mode) {
    default:
    case Mode::Run:
        return std::unexpected(Error::create(ErrorKind::InternalError, SourceSpan(0, 0),
                                             "should not reach end of vm::run without exit"));
    case Mode::Repl:
        if (ptr >= 1) {
            print_value(call_stack.back().registers[proto.bytecode[ptr - 1]]);
            std::cout << std::endl;
        }
        return {};
    }
}

std::expected<void, Error> VM::call(std::shared_ptr<Proto> fn, uint16_t offset) {
    // first, load the arguments into a list:
    Values vs;
    for (size_t i = 0; i < fn->arguments; ++i) {
        // start at offset + i.
        vs.push_back(call_stack.back().registers[i + offset]);
    }

    // then, if the function is pure, check the cache:
    if (fn->is_pure) {
        if (auto hit = fn->cache.get(vs)) {
            // handle the cache hit. load the return value to register 0 of this call frame and
            // return.
            call_stack[call_stack.size() - 1].registers[0] = *hit;
            return {};
        }
    }

    // if we are here, we will have to run the function. begin by creating the next call
    // frame and preparing it with the arguments:
    std::array<Value, 256> registers;
    for (size_t i = 0; i < fn->arguments; ++i) {
        // start at +1 + i index.
        registers[i + 1] = vs[i];
    }
    CallFrame frame{std::move(registers), 0.0};
    call_stack.push_back(frame);

    // first check whether we have a compiled version and if we do, run that:
    if (fn->compiled) [[unlikely]] {
        TY_TRACE("running compiled machine code");
        call_stack[call_stack.size() - 2].registers[0] =
            reinterpret_cast<double (*)()>(fn->compiled)();
        TY_TRACE(reinterpret_cast<double (*)()>(fn->compiled)());
        call_stack.pop_back();
        return {};
    }

    // if we are here, then we will have to interpret the function.
    if (auto post_fn = run(*fn); !post_fn) return std::unexpected(post_fn.error());

    // load the return value of the function to register 0 of this
    // call frame.
    // TODO: consider just throwing this in the returns slot instead
    //  of registers[0], so that the 0th slot can be used for other
    //  things. blocked by bytecode generation.
    call_stack[call_stack.size() - 2].registers[0] = call_stack.back().returns;

    // if the function is pure, put result in cache.
    if (fn->is_pure) {
        fn->cache.put(vs, call_stack.back().returns);
    }

    // if we hit the counter, generate machine code
    // TODO: for now I'm using the is_pure indicator, but this should really have its own indicator.
    if (fn->is_pure && ++fn->compilation_counter >= 10) {
        auto arena = codegen::generate_ir(fn);
        if (!arena) return std::unexpected(arena.error());
        auto codegen = codegen::generate_machine(*arena, fn, call_stack.back().registers.data());
        if (!codegen) return std::unexpected(codegen.error());
    }

    // remove call frame.
    call_stack.pop_back();

    return {};
}

void VM::doctor() const {
    std::println("vm state:");
    for (const CallFrame &call : call_stack) {
        for (const Value &reg : call.registers) {
            print_value(reg);
            std::print(" ");
        }
        std::print("\nreturned: ");
        print_value(call.returns);
        std::println();
    }
}
} // namespace tachyon::runtime
