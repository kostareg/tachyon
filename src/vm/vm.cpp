module;

#include <cmath>
#include <expected>
#include <iostream>
#include <iterator>

module vm;

namespace vm {
std::expected<void, Error> VM::run(const Proto &proto) {
    size_t ptr = 0;
    while (ptr < proto.bytecode.size()) {
        uint8_t op = proto.bytecode[ptr];
        std::println("0x{:02X}", op);

        switch (op) {
        default:
        case EXIT:
            return {};
        case NOOP: {
            ++ptr;
            continue;
        }
        case RETV:
        case RETC:
        case RETR: {
            ++ptr;
            continue; // TODO: implement
        }
        case LOCR: {
            auto src1 = proto.bytecode[++ptr];
            auto dst = proto.bytecode[++ptr];
            call_stack.back().registers[dst] = proto.constants[src1];
            break;
        }
        case LORR: {
            auto src1 = proto.bytecode[++ptr];
            auto dst = proto.bytecode[++ptr];
            call_stack.back().registers[dst] =
                call_stack.back().registers[src1];
            break;
        }
        case CRLC: {
            auto src1 = proto.bytecode[++ptr];
            auto src2 = proto.bytecode[++ptr];
            auto dst = proto.bytecode[++ptr];
            call_stack.back().registers[dst] =
                call_stack.back().registers[src1] < proto.constants[src2];
            break;
        }
        case CRGC: {
            auto src1 = proto.bytecode[++ptr];
            auto src2 = proto.bytecode[++ptr];
            auto dst = proto.bytecode[++ptr];
            call_stack.back().registers[dst] =
                call_stack.back().registers[src1] > proto.constants[src2];
            break;
        }
        case CRLR: {
            auto src1 = proto.bytecode[++ptr];
            auto src2 = proto.bytecode[++ptr];
            auto dst = proto.bytecode[++ptr];
            call_stack.back().registers[dst] =
                call_stack.back().registers[src1] <
                call_stack.back().registers[src2];
            break;
        }
        case CRGR: {
            auto src1 = proto.bytecode[++ptr];
            auto src2 = proto.bytecode[++ptr];
            auto dst = proto.bytecode[++ptr];
            call_stack.back().registers[dst] =
                call_stack.back().registers[src1] >
                call_stack.back().registers[src2];
            break;
        }
        case CREC: {
            auto src1 = proto.bytecode[++ptr];
            auto src2 = proto.bytecode[++ptr];
            auto dst = proto.bytecode[++ptr];
            call_stack.back().registers[dst] =
                call_stack.back().registers[src1] == proto.constants[src2];
            break;
        }
        case CRER: {
            auto src1 = proto.bytecode[++ptr];
            auto src2 = proto.bytecode[++ptr];
            auto dst = proto.bytecode[++ptr];
            call_stack.back().registers[dst] =
                call_stack.back().registers[src1] ==
                call_stack.back().registers[src2];
            break;
        }
        // TODO: if you implement integer type, use that in JMPC/JMPR instead
        case JMPC: {
            auto src1 = proto.bytecode[++ptr];
            ptr = std::get<double>(proto.constants[src1]);
            continue; // do not increment
        }
        case JMPR: {
            auto src1 = proto.bytecode[++ptr];
            ptr = std::get<double>(call_stack.back().registers[src1]);
            continue;
        }
        // TODO: for now, just assume that math operations are only working on
        //  doubles. we can handle other issues either in the typechecker or by
        //  allowing weak types.
        case MARC: {
            auto src1 = proto.bytecode[++ptr];
            auto src2 = proto.bytecode[++ptr];
            auto dst = proto.bytecode[++ptr];
            call_stack.back().registers[dst] =
                std::get<double>(call_stack.back().registers[src1]) +
                std::get<double>(proto.constants[src2]);
            break;
        }
        case MSRC: {
            auto src1 = proto.bytecode[++ptr];
            auto src2 = proto.bytecode[++ptr];
            auto dst = proto.bytecode[++ptr];
            call_stack.back().registers[dst] =
                std::get<double>(call_stack.back().registers[src1]) -
                std::get<double>(proto.constants[src2]);
            break;
        }
        case MMRC: {
            auto src1 = proto.bytecode[++ptr];
            auto src2 = proto.bytecode[++ptr];
            auto dst = proto.bytecode[++ptr];
            call_stack.back().registers[dst] =
                std::get<double>(call_stack.back().registers[src1]) *
                std::get<double>(proto.constants[src2]);
            break;
        }
        case MDRC: {
            auto src1 = proto.bytecode[++ptr];
            auto src2 = proto.bytecode[++ptr];
            auto dst = proto.bytecode[++ptr];
            call_stack.back().registers[dst] =
                std::get<double>(call_stack.back().registers[src1]) /
                std::get<double>(proto.constants[src2]);
            break;
        }
        case MPRC: {
            auto src1 = proto.bytecode[++ptr];
            auto src2 = proto.bytecode[++ptr];
            auto dst = proto.bytecode[++ptr];
            call_stack.back().registers[dst] =
                pow(std::get<double>(call_stack.back().registers[src1]),
                    std::get<double>(proto.constants[src2]));
            break;
        }
        case MACR: {
            auto src1 = proto.bytecode[++ptr];
            auto src2 = proto.bytecode[++ptr];
            auto dst = proto.bytecode[++ptr];
            call_stack.back().registers[dst] =
                std::get<double>(proto.constants[src1]) +
                std::get<double>(call_stack.back().registers[src2]);
            break;
        }
        case MSCR: {
            auto src1 = proto.bytecode[++ptr];
            auto src2 = proto.bytecode[++ptr];
            auto dst = proto.bytecode[++ptr];
            call_stack.back().registers[dst] =
                std::get<double>(proto.constants[src1]) -
                std::get<double>(call_stack.back().registers[src2]);
            break;
        }
        case MMCR: {
            auto src1 = proto.bytecode[++ptr];
            auto src2 = proto.bytecode[++ptr];
            auto dst = proto.bytecode[++ptr];
            call_stack.back().registers[dst] =
                std::get<double>(proto.constants[src1]) *
                std::get<double>(call_stack.back().registers[src2]);
            break;
        }
        case MDCR: {
            auto src1 = proto.bytecode[++ptr];
            auto src2 = proto.bytecode[++ptr];
            auto dst = proto.bytecode[++ptr];
            call_stack.back().registers[dst] =
                std::get<double>(proto.constants[src1]) /
                std::get<double>(call_stack.back().registers[src2]);
            break;
        }
        case MPCR: {
            auto src1 = proto.bytecode[++ptr];
            auto src2 = proto.bytecode[++ptr];
            auto dst = proto.bytecode[++ptr];
            call_stack.back().registers[dst] =
                pow(std::get<double>(proto.constants[src1]),
                    std::get<double>(call_stack.back().registers[src2]));
            break;
        }
        case MARR: {
            auto src1 = proto.bytecode[++ptr];
            auto src2 = proto.bytecode[++ptr];
            auto dst = proto.bytecode[++ptr];
            call_stack.back().registers[dst] =
                std::get<double>(call_stack.back().registers[src1]) +
                std::get<double>(call_stack.back().registers[src2]);
            break;
        }
        case MSRR: {
            auto src1 = proto.bytecode[++ptr];
            auto src2 = proto.bytecode[++ptr];
            auto dst = proto.bytecode[++ptr];
            call_stack.back().registers[dst] =
                std::get<double>(call_stack.back().registers[src1]) -
                std::get<double>(call_stack.back().registers[src2]);
            break;
        }
        case MMRR: {
            auto src1 = proto.bytecode[++ptr];
            auto src2 = proto.bytecode[++ptr];
            auto dst = proto.bytecode[++ptr];
            call_stack.back().registers[dst] =
                std::get<double>(call_stack.back().registers[src1]) *
                std::get<double>(call_stack.back().registers[src2]);
            break;
        }
        case MDRR: {
            auto src1 = proto.bytecode[++ptr];
            auto src2 = proto.bytecode[++ptr];
            auto dst = proto.bytecode[++ptr];
            call_stack.back().registers[dst] =
                std::get<double>(call_stack.back().registers[src1]) /
                std::get<double>(call_stack.back().registers[src2]);
            break;
        }
        case MPRR: {
            auto src1 = proto.bytecode[++ptr];
            auto src2 = proto.bytecode[++ptr];
            auto dst = proto.bytecode[++ptr];
            call_stack.back().registers[dst] =
                pow(std::get<double>(call_stack.back().registers[src1]),
                    std::get<double>(call_stack.back().registers[src2]));
            break;
        }
        }
        ++ptr;
    }

    return std::unexpected(Error(ErrorKind::InternalError,
                                 "should not reach end of vm::run without exit",
                                 0, 0, 0, 0));
}

void VM::diagnose() const {
    std::println("vm state:");
    for (auto &call : call_stack) {
        for (auto &reg : call.registers) {
            std::visit(
                [](const auto &val) {
                    using T = std::decay_t<decltype(val)>;
                    if constexpr (std::is_same_v<T, std::monostate>) {
                        std::print("() ");
                    } else if constexpr (std::is_same_v<T, const Proto *>) {
                        std::print("proto ");
                    } else {
                        std::print("{} ", val);
                    }
                },
                reg);
        }
        std::println("\nreturned: {}", call.returns);
    }
}
} // namespace vm
