#include <cmath>
#include <iostream>
#include <spdlog/spdlog.h>

#include "vm/vm.hpp"

namespace vm {
void VM::run_fn(Proto *proto) {
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
        return;
    }

    while (1) {
        uint16_t op = proto->bc[pc];
        spdlog::trace("{}[{}] op 0x{:04x}", proto->name, pc, op);
        ++pc;

        switch (op) {
        // exit (defined in while condition)
        case 0x0000:
        // no-op
        case 0x0001:
        // return void
        case 0x0002:
            break;
        // return const
        case 0x0003:
            registers[0] = proto->bc[pc];
            break;
        // return reg
        case 0x0004:
            registers[0] = registers[proto->bc[pc]];
            break;
        // load reg from const
        case 0x0010: {
            uint16_t reg = proto->bc[pc++];
            uint16_t val = proto->bc[pc++];
            registers[reg] = val;
            break;
        };
        // load reg from reg
        case 0x0011: {
            uint16_t reg = proto->bc[pc++];
            uint16_t val = registers[proto->bc[pc++]];
            registers[reg] = val;
            break;
        };
        // load reg from top of stack
        case 0x0012: {
            uint16_t reg = proto->bc[pc++];
            uint16_t val = stack.top();
            registers[reg] = val;
            break;
        };
        // push const to stack
        case 0x0020: {
            uint16_t val = proto->bc[pc++];
            stack.push(val);
            break;
        };
        // push reg to stack
        case 0x0021: {
            uint16_t val = registers[proto->bc[pc++]];
            stack.push(val);
            break;
        };
        // pop stack
        case 0x002E: {
            stack.pop();
            break;
        }
        // reg < const -> reg
        case 0x0030: {
            uint16_t l = registers[proto->bc[pc++]];
            uint16_t r = proto->bc[pc++];
            registers[proto->bc[pc++]] = l < r;
            break;
        };
        // reg > const -> reg
        case 0x0031: {
            uint16_t l = registers[proto->bc[pc++]];
            uint16_t r = registers[proto->bc[pc++]];
            registers[proto->bc[pc++]] = l > r;
            break;
        };
        // reg < reg -> reg
        case 0x0032: {
            uint16_t l = registers[proto->bc[pc++]];
            uint16_t r = proto->bc[pc++];
            registers[proto->bc[pc++]] = l < r;
            break;
        };
        // reg > reg -> reg
        case 0x0033: {
            uint16_t l = registers[proto->bc[pc++]];
            uint16_t r = registers[proto->bc[pc++]];
            registers[proto->bc[pc++]] = l > r;
            break;
        };
        // reg == const -> reg
        case 0x0034: {
            uint16_t l = registers[proto->bc[pc++]];
            uint16_t r = proto->bc[pc++];
            registers[proto->bc[pc++]] = l == r;
            break;
        };
        // reg == reg -> reg
        case 0x0035: {
            uint16_t l = registers[proto->bc[pc++]];
            uint16_t r = registers[proto->bc[pc++]];
            registers[proto->bc[pc++]] = l == r;
            break;
        };
        // jump const
        case 0x0040: {
            pc = proto->bc[pc];
            break;
        };
        // jump reg
        case 0x0041: {
            pc = registers[proto->bc[pc]];
            break;
        };
        // jump from top stack
        case 0x0042: {
            pc = stack.top();
            break;
        };
        // TODO: if reg then jump
        // reg + const -> reg
        case 0x0050: {
            uint16_t l = registers[proto->bc[pc++]];
            uint16_t r = proto->bc[pc++];
            registers[proto->bc[pc++]] = l + r;
            break;
        };
        // reg - const -> reg
        case 0x0051: {
            uint16_t l = registers[proto->bc[pc++]];
            uint16_t r = proto->bc[pc++];
            registers[proto->bc[pc++]] = l - r;
            break;
        };
        // reg * const -> reg
        case 0x0052: {
            uint16_t l = registers[proto->bc[pc++]];
            uint16_t r = proto->bc[pc++];
            registers[proto->bc[pc++]] = l * r;
            break;
        };
        // reg / const -> reg
        case 0x0053: {
            uint16_t l = registers[proto->bc[pc++]];
            uint16_t r = proto->bc[pc++];
            registers[proto->bc[pc++]] = l / r;
            break;
        };
        // reg ^ const -> reg
        case 0x0054: {
            uint16_t l = registers[proto->bc[pc++]];
            uint16_t r = proto->bc[pc++];
            registers[proto->bc[pc++]] = pow(l, r);
            break;
        };
        // const + reg -> reg
        case 0x0055: {
            uint16_t l = proto->bc[pc++];
            uint16_t r = registers[proto->bc[pc++]];
            registers[proto->bc[pc++]] = l + r;
            break;
        };
        // const - reg -> reg
        case 0x0056: {
            uint16_t l = proto->bc[pc++];
            uint16_t r = registers[proto->bc[pc++]];
            registers[proto->bc[pc++]] = l - r;
            break;
        };
        // const * reg -> reg
        case 0x0057: {
            uint16_t l = proto->bc[pc++];
            uint16_t r = registers[proto->bc[pc++]];
            registers[proto->bc[pc++]] = l * r;
            break;
        };
        // const / reg -> reg
        case 0x0058: {
            uint16_t l = proto->bc[pc++];
            uint16_t r = registers[proto->bc[pc++]];
            registers[proto->bc[pc++]] = l / r;
            break;
        };
        // const ^ reg -> reg
        case 0x0059: {
            uint16_t l = proto->bc[pc++];
            uint16_t r = registers[proto->bc[pc++]];
            registers[proto->bc[pc++]] = pow(l, r);
            break;
        };
        // reg + reg -> reg
        case 0x005A: {
            uint16_t l = registers[proto->bc[pc++]];
            uint16_t r = registers[proto->bc[pc++]];
            registers[proto->bc[pc++]] = l + r;
            break;
        };
        // reg - reg -> reg
        case 0x005B: {
            uint16_t l = registers[proto->bc[pc++]];
            uint16_t r = registers[proto->bc[pc++]];
            registers[proto->bc[pc++]] = l - r;
            break;
        };
        // reg * reg -> reg
        case 0x005C: {
            uint16_t l = registers[proto->bc[pc++]];
            uint16_t r = registers[proto->bc[pc++]];
            registers[proto->bc[pc++]] = l * r;
            break;
        };
        // reg / reg -> reg
        case 0x005D: {
            uint16_t l = registers[proto->bc[pc++]];
            uint16_t r = registers[proto->bc[pc++]];
            registers[proto->bc[pc++]] = l / r;
            break;
        };
        // reg ^ reg -> reg
        case 0x005E: {
            uint16_t l = registers[proto->bc[pc++]];
            uint16_t r = registers[proto->bc[pc++]];
            registers[proto->bc[pc++]] = pow(l, r);
            break;
        };
        // call reg
        case 0x0100: {
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
            run_fn(fn);

            if (fn->returns)
                registers.get_last_fn(0) = registers[0];

            registers.free_fn();

            break;
        };
        // push const fn param
        case 0x0110: {
            uint16_t idx = proto->bc[pc++];
            size_t val = proto->bc[pc++];
            registers.early_push(idx, val);
            break;
        };
        // push reg fn param
        case 0x0111: {
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
}
} // namespace vm
