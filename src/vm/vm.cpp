#include <cmath>
#include <iostream>
#include <spdlog/spdlog.h>

#include "bytecode.hpp"
#include "vm/vm.hpp"

namespace vm {
std::expected<void, Error> VM::run_fn(Proto *proto) {
    spdlog::trace("running function {}", proto->name);

    size_t numChildren = proto->children.size();
    spdlog::trace("with {} children", numChildren);

    // load protos to vm
    if (numChildren) {
        fns.insert(fns.end(), std::make_move_iterator(proto->children.begin()),
                   std::make_move_iterator(proto->children.end()));
        proto->children.clear();
    }

    registers.new_fn();
    uint16_t pc = 0;

    if (!proto->bc) {
        std::cerr << "ice bytecode uninitialized" << std::endl;
        return {};
    }

    while (1) {
        uint16_t op = proto->bc[pc];
        spdlog::trace("{}[{}] op 0x{:04x}", proto->name, pc, op);
        ++pc;

        switch (op) {
        // exit (defined in while condition)
        case EXIT:
        // no-op
        case NOOP:
        // return void
        case RETV:
            break;
        // return const
        case RETC:
            registers[0] = proto->bc[pc];
            break;
        // return reg
        case RETR:
            registers[0] = registers[proto->bc[pc]];
            break;
        // load reg from const
        case LORC: {
            uint16_t reg = proto->bc[pc++];
            uint16_t val = proto->bc[pc++];
            registers[reg] = val;
            break;
        };
        // load reg from reg
        case LORR: {
            uint16_t reg = proto->bc[pc++];
            uint16_t val = registers[proto->bc[pc++]];
            registers[reg] = val;
            break;
        };
        // load reg from top of stack
        case LORS: {
            uint16_t reg = proto->bc[pc++];
            uint16_t val = stack.top();
            registers[reg] = val;
            break;
        };
        // push const to stack
        case PUSC: {
            uint16_t val = proto->bc[pc++];
            stack.push(val);
            break;
        };
        // push reg to stack
        case PUSR: {
            uint16_t val = registers[proto->bc[pc++]];
            stack.push(val);
            break;
        };
        // pop stack
        case POPS: {
            stack.pop();
            break;
        }
        // reg < const -> reg
        case CRLC: {
            uint16_t l = registers[proto->bc[pc++]];
            uint16_t r = proto->bc[pc++];
            registers[proto->bc[pc++]] = l < r;
            break;
        };
        // reg > const -> reg
        case CRGC: {
            uint16_t l = registers[proto->bc[pc++]];
            uint16_t r = registers[proto->bc[pc++]];
            registers[proto->bc[pc++]] = l > r;
            break;
        };
        // reg < reg -> reg
        case CRLR: {
            uint16_t l = registers[proto->bc[pc++]];
            uint16_t r = proto->bc[pc++];
            registers[proto->bc[pc++]] = l < r;
            break;
        };
        // reg > reg -> reg
        case CRGR: {
            uint16_t l = registers[proto->bc[pc++]];
            uint16_t r = registers[proto->bc[pc++]];
            registers[proto->bc[pc++]] = l > r;
            break;
        };
        // reg == const -> reg
        case CREC: {
            uint16_t l = registers[proto->bc[pc++]];
            uint16_t r = proto->bc[pc++];
            registers[proto->bc[pc++]] = l == r;
            break;
        };
        // reg == reg -> reg
        case CRER: {
            uint16_t l = registers[proto->bc[pc++]];
            uint16_t r = registers[proto->bc[pc++]];
            registers[proto->bc[pc++]] = l == r;
            break;
        };
        // jump const
        case JMPC: {
            pc = proto->bc[pc];
            break;
        };
        // jump reg
        case JMPR: {
            pc = registers[proto->bc[pc]];
            break;
        };
        // jump from top stack
        case JMPS: {
            pc = stack.top();
            break;
        };
        // TODO: if reg then jump
        // reg + const -> reg
        case MARC: {
            uint16_t l = registers[proto->bc[pc++]];
            uint16_t r = proto->bc[pc++];
            registers[proto->bc[pc++]] = l + r;
            break;
        };
        // reg - const -> reg
        case MSRC: {
            uint16_t l = registers[proto->bc[pc++]];
            uint16_t r = proto->bc[pc++];
            registers[proto->bc[pc++]] = l - r;
            break;
        };
        // reg * const -> reg
        case MMRC: {
            uint16_t l = registers[proto->bc[pc++]];
            uint16_t r = proto->bc[pc++];
            registers[proto->bc[pc++]] = l * r;
            break;
        };
        // reg / const -> reg
        case MDRC: {
            uint16_t l = registers[proto->bc[pc++]];
            uint16_t r = proto->bc[pc++];
            registers[proto->bc[pc++]] = l / r;
            break;
        };
        // reg ^ const -> reg
        case MPRC: {
            uint16_t l = registers[proto->bc[pc++]];
            uint16_t r = proto->bc[pc++];
            registers[proto->bc[pc++]] = pow(l, r);
            break;
        };
        // const + reg -> reg
        case MACR: {
            uint16_t l = proto->bc[pc++];
            uint16_t r = registers[proto->bc[pc++]];
            registers[proto->bc[pc++]] = l + r;
            break;
        };
        // const - reg -> reg
        case MSCR: {
            uint16_t l = proto->bc[pc++];
            uint16_t r = registers[proto->bc[pc++]];
            registers[proto->bc[pc++]] = l - r;
            break;
        };
        // const * reg -> reg
        case MMCR: {
            uint16_t l = proto->bc[pc++];
            uint16_t r = registers[proto->bc[pc++]];
            registers[proto->bc[pc++]] = l * r;
            break;
        };
        // const / reg -> reg
        case MDCR: {
            uint16_t l = proto->bc[pc++];
            uint16_t r = registers[proto->bc[pc++]];
            registers[proto->bc[pc++]] = l / r;
            break;
        };
        // const ^ reg -> reg
        case MPCR: {
            uint16_t l = proto->bc[pc++];
            uint16_t r = registers[proto->bc[pc++]];
            registers[proto->bc[pc++]] = pow(l, r);
            break;
        };
        // reg + reg -> reg
        case MARR: {
            uint16_t l = registers[proto->bc[pc++]];
            uint16_t r = registers[proto->bc[pc++]];
            registers[proto->bc[pc++]] = l + r;
            break;
        };
        // reg - reg -> reg
        case MSRR: {
            uint16_t l = registers[proto->bc[pc++]];
            uint16_t r = registers[proto->bc[pc++]];
            registers[proto->bc[pc++]] = l - r;
            break;
        };
        // reg * reg -> reg
        case MMRR: {
            uint16_t l = registers[proto->bc[pc++]];
            uint16_t r = registers[proto->bc[pc++]];
            registers[proto->bc[pc++]] = l * r;
            break;
        };
        // reg / reg -> reg
        case MDRR: {
            uint16_t l = registers[proto->bc[pc++]];
            uint16_t r = registers[proto->bc[pc++]];
            registers[proto->bc[pc++]] = l / r;
            break;
        };
        // reg ^ reg -> reg
        case MPRR: {
            uint16_t l = registers[proto->bc[pc++]];
            uint16_t r = registers[proto->bc[pc++]];
            registers[proto->bc[pc++]] = pow(l, r);
            break;
        };
        // call reg
        case CALR: {
            uint16_t idx = registers[proto->bc[pc++]];

            spdlog::trace("calling fn register {}", idx);

            if (idx == 0xFFFF) {
                spdlog::trace("print {}", registers[256]);
                // special case: print register (param)
                std::cout << registers[256] << std::endl;
                break;
            } else if (idx >= fns.size()) {
                // if its not a parent or child function
                throw std::runtime_error("ice: unknown fn register");
                break;
            }

            auto fn = fns[idx].get();
            auto r = run_fn(fn);
            if (!r.has_value()) {
                return std::unexpected(r.error());
            }

            if (fn->returns)
                registers.get_last_fn(0) = registers[0];

            registers.free_fn();

            break;
        };
        // push const fn param
        case PUHC: {
            uint16_t idx = proto->bc[pc++];
            size_t val = proto->bc[pc++];
            registers.early_push(idx, val);
            break;
        };
        // push reg fn param
        case PUHR: {
            uint16_t idx = proto->bc[pc++];
            size_t val = registers[proto->bc[pc++]];
            registers.early_push(idx, val);
            break;
        };
        default:
            std::cerr << "unknown instruction " << op << std::endl;
            break;
        }

        // if returning from the fn, break
        if (op == 0x0000 || op == 0x0001 || op == 0x0002 || op == 0x0003 ||
            op == 0x0004)
            break;
    }

    spdlog::trace("{} done, regalloc: {}, {}, {}", proto->name, registers[0],
                  registers[1], registers[2]);

    // move children back for next use
    if (numChildren) {
        spdlog::trace("moving {} children of {}", numChildren, proto->name);
        proto->children.insert(proto->children.end(),
                               std::make_move_iterator(fns.end() - numChildren),
                               std::make_move_iterator(fns.end()));
        fns.erase(fns.end() - numChildren, fns.end());
    }

    return {};
}
} // namespace vm
