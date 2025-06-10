module;

#include <cmath>
#include <expected>
#include <iostream>
#include <iterator>

module vm;

namespace vm {
/**
 * @brief small visitor for printing values
 */
void print_value(const Value &reg) {
    std::visit(
        [](const auto &val) {
            using T = std::decay_t<decltype(val)>;
            if constexpr (std::is_same_v<T, std::monostate>) {
                std::print("()");
            } else if constexpr (std::is_same_v<T, std::shared_ptr<Proto>>) {
                std::print("proto");
            } else {
                std::print("{}", val);
            }
        },
        reg);
}

std::expected<void, Error> VM::run(const Proto &proto) {
    size_t ptr = 0;
    while (ptr < proto.bytecode.size()) {
        uint8_t op = proto.bytecode[ptr];
        // std::println("0x{:02X}", op);

        switch (op) {
        default:
        case RETV: {
            return {};
        }
        case RETC: {
            auto src1 = proto.bytecode[++ptr];
            call_stack.back().returns = proto.constants[src1];
            return {};
        }
        case RETR: {
            auto src1 = proto.bytecode[++ptr];
            call_stack.back().returns = call_stack.back().registers[src1];
            return {};
        }
        // TODO: if the section after this switch case remains blank, don't
        //  continue, just break.
        case NOOP: {
            ++ptr;
            continue;
        }
        // TODO: consider putting a check here that makes sure constants exists.
        //  otherwise, its a segfault.
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
        //  allowing weak types. or, we can make it a runtime error.
        case MACC: {
            auto src1 = proto.bytecode[++ptr];
            auto src2 = proto.bytecode[++ptr];
            auto dst = proto.bytecode[++ptr];
            call_stack.back().registers[dst] =
                std::get<double>(proto.constants[src1]) +
                std::get<double>(proto.constants[src2]);
            break;
        }
        case MSCC: {
            auto src1 = proto.bytecode[++ptr];
            auto src2 = proto.bytecode[++ptr];
            auto dst = proto.bytecode[++ptr];
            call_stack.back().registers[dst] =
                std::get<double>(proto.constants[src1]) -
                std::get<double>(proto.constants[src2]);
            break;
        }
        case MMCC: {
            auto src1 = proto.bytecode[++ptr];
            auto src2 = proto.bytecode[++ptr];
            auto dst = proto.bytecode[++ptr];
            call_stack.back().registers[dst] =
                std::get<double>(proto.constants[src1]) *
                std::get<double>(proto.constants[src2]);
            break;
        }
        case MDCC: {
            auto src1 = proto.bytecode[++ptr];
            auto src2 = proto.bytecode[++ptr];
            auto dst = proto.bytecode[++ptr];
            call_stack.back().registers[dst] =
                std::get<double>(proto.constants[src1]) /
                std::get<double>(proto.constants[src2]);
            break;
        }
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
        case CALC: {
            auto src1 = proto.bytecode[++ptr];
            auto offset = proto.bytecode[++ptr];
            auto fn = std::get<std::shared_ptr<Proto>>(proto.constants[src1]);

            if (auto calledFn = call(fn, offset); !calledFn)
                return std::unexpected(calledFn.error());
            break;
        }
        case CALR: {
            auto src1 = proto.bytecode[++ptr];
            auto offset = proto.bytecode[++ptr];
            auto fn = std::get<std::shared_ptr<Proto>>(
                call_stack.back().registers[src1]);

            if (auto calledFn = call(fn, offset); !calledFn)
                return std::unexpected(calledFn.error());
            break;
        }
        case PRNC: {
            auto src1 = proto.bytecode[++ptr];
            print_value(proto.constants[src1]);
            break;
        }
        case PRNR: {
            auto src1 = proto.bytecode[++ptr];
            print_value(call_stack.back().registers[src1]);
            break;
        }
        }
        ++ptr;
    }

    return std::unexpected(Error(ErrorKind::InternalError,
                                 "should not reach end of vm::run without exit",
                                 0, 0, 0, 0));
}

std::expected<void, Error> VM::call(std::shared_ptr<Proto> fn, uint8_t offset) {
    // create the next call frame and prepare it with the first few
    // registers of the old ones.
    std::array<Value, 256> registers;
    for (size_t i = 0; i < fn->arguments; ++i) {
        // start at +1 + i <- offset + i
        registers[i + 1] = call_stack.back().registers[i + offset];
    }
    CallFrame frame{std::move(registers)};
    call_stack.push_back(frame);

    if (auto postFn = run(*fn); !postFn)
        return std::unexpected(postFn.error());

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

void VM::diagnose() const {
    std::println("vm state:");
    for (auto &call : call_stack) {
        for (auto &reg : call.registers) {
            print_value(reg);
            std::print(" ");
        }
        std::print("\nreturned: ");
        print_value(call.returns);
        std::println();
    }
}
} // namespace vm
