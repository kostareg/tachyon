#include "vm.hpp"

#include <cmath>
#include <iostream>

void VM::load(uint16_t program[0xFFFF]) {
    std::copy(program, program + 0xFFFF, memory);
}

void VM::run() {
    while (true) {
        uint16_t op = memory[pc];

        std::cout << pc << " : " << op << std::endl;
        ++pc;

        switch (op) {
            // halt
            case 0: return;
            // no-op
            case 1: break;
            // load reg from const
            case 2: {
                uint16_t reg = memory[pc];
                ++pc;
                uint16_t val = memory[pc];
                ++pc;
                registers[reg] = val;
                break;
            };
            // load reg from reg
            case 3: {
                uint16_t reg = memory[pc];
                ++pc;
                uint16_t val = registers[memory[pc]];
                ++pc;
                registers[reg] = val;
                break;
            };
            // load reg from memory
            case 4: {
                uint16_t reg = memory[pc];
                ++pc;
                uint16_t val = memory[memory[pc]];
                ++pc;
                registers[reg] = val;
                break;
            };
            // load reg from popped stack
            case 5: {
                uint16_t reg = memory[pc];
                ++pc;
                uint16_t val = stack.top();
                stack.pop();
                registers[reg] = val;
                break;
            };
            // load memory from const
            case 6: {
                uint16_t addr = memory[pc];
                ++pc;
                uint16_t val = memory[pc];
                ++pc;
                memory[addr] = val;
                break;
            };
            // load memory from reg
            case 7: {
                uint16_t addr = memory[pc];
                ++pc;
                uint16_t val = registers[memory[pc]];
                ++pc;
                memory[addr] = val;
                break;
            };
            // load memory from memory
            case 8: {
                uint16_t addr = memory[pc];
                ++pc;
                uint16_t val = memory[memory[pc]];
                ++pc;
                memory[addr] = val;
                break;
            };
            // load memory from popped stack
            case 9: {
                uint16_t addr = memory[pc];
                ++pc;
                uint16_t val = stack.top();
                stack.pop();
                memory[addr] = val;
                break;
            };
            // push const to stack
            case 10: {
                uint16_t val = memory[pc];
                ++pc;
                stack.push(val);
                break;
            };
            // push reg to stack
            case 11: {
                uint16_t val = registers[memory[pc]];
                ++pc;
                stack.push(val);
                break;
            };
            // push memory to stack
            case 12: {
                uint16_t val = memory[memory[pc]];
                ++pc;
                stack.push(val);
                break;
            };
            // reg + reg -> reg
            case 13: {
                uint16_t l = registers[memory[pc]];
                ++pc;
                uint16_t r = registers[memory[pc]];
                ++pc;
                registers[memory[pc]] = l + r;
                ++pc;
                break;
            };
            // reg - reg -> reg
            case 14: {
                uint16_t l = registers[memory[pc]];
                ++pc;
                uint16_t r = registers[memory[pc]];
                ++pc;
                registers[memory[pc]] = l - r;
                ++pc;
                break;
            };
            // reg * reg -> reg
            case 15: {
                uint16_t l = registers[memory[pc]];
                ++pc;
                uint16_t r = registers[memory[pc]];
                ++pc;
                registers[memory[pc]] = l * r;
                ++pc;
                break;
            };
            // reg / reg -> reg
            case 16: {
                uint16_t l = registers[memory[pc]];
                ++pc;
                uint16_t r = registers[memory[pc]];
                ++pc;
                registers[memory[pc]] = l / r;
                ++pc;
                break;
            };
            // reg ^ reg -> reg
            case 17: {
                uint16_t l = registers[memory[pc]];
                ++pc;
                uint16_t r = registers[memory[pc]];
                ++pc;
                registers[memory[pc]] = pow(l, r);
                ++pc;
                break;
            };
            // reg + const -> reg
            case 18: {
                uint16_t l = registers[memory[pc]];
                ++pc;
                uint16_t r = memory[pc];
                ++pc;
                registers[memory[pc]] = l + r;
                ++pc;
                break;
            };
            // reg - const -> reg
            case 19: {
                uint16_t l = registers[memory[pc]];
                ++pc;
                uint16_t r = memory[pc];
                ++pc;
                registers[memory[pc]] = l - r;
                ++pc;
                break;
            };
            // reg * const -> reg
            case 20: {
                uint16_t l = registers[memory[pc]];
                ++pc;
                uint16_t r = memory[pc];
                ++pc;
                registers[memory[pc]] = l * r;
                ++pc;
                break;
            };
            // reg / const -> reg
            case 21: {
                uint16_t l = registers[memory[pc]];
                ++pc;
                uint16_t r = memory[pc];
                ++pc;
                registers[memory[pc]] = l / r;
                ++pc;
                break;
            };
            // reg ^ const -> reg
            case 22: {
                uint16_t l = registers[memory[pc]];
                ++pc;
                uint16_t r = memory[pc];
                ++pc;
                registers[memory[pc]] = pow(l, r);
                ++pc;
                break;
            };
            // const - reg -> reg
            case 23: {
                uint16_t l = memory[pc];
                ++pc;
                uint16_t r = registers[memory[pc]];
                ++pc;
                registers[memory[pc]] = l - r;
                ++pc;
                break;
            };
            // const / reg -> reg
            case 24: {
                uint16_t l = memory[pc];
                ++pc;
                uint16_t r = registers[memory[pc]];
                ++pc;
                registers[memory[pc]] = l / r;
                ++pc;
                break;
            };
            // const ^ reg -> reg
            case 25: {
                uint16_t l = memory[pc];
                ++pc;
                uint16_t r = registers[memory[pc]];
                ++pc;
                registers[memory[pc]] = pow(l, r);
                ++pc;
                break;
            };
            // jump const
            case 26: {
                pc = memory[pc];
                break;
            };
            // jump reg
            case 27: {
                pc = registers[memory[pc]];
                break;
            };
            // jump memory
            case 28: {
                pc = memory[memory[pc]];
                break;
            };
            // jump pop stack
            case 29: {
                pc = stack.top();
                stack.pop();
                break;
            };
            // reg eq const -> reg
            case 30: {
                uint16_t l = registers[memory[pc]];
                ++pc;
                uint16_t r = memory[pc];
                ++pc;
                registers[memory[pc]] = l == r;
                ++pc;
                break;
            };
            // reg eq reg -> reg
            case 31: {
                uint16_t l = registers[memory[pc]];
                ++pc;
                uint16_t r = registers[memory[pc]];
                ++pc;
                registers[memory[pc]] = l == r;
                ++pc;
                break;
            };
            // reg < const -> reg
            case 32: {
                uint16_t l = registers[memory[pc]];
                ++pc;
                uint16_t r = memory[pc];
                ++pc;
                registers[memory[pc]] = l < r;
                ++pc;
                break;
            };
            // reg > const -> reg
            case 33: {
                uint16_t l = registers[memory[pc]];
                ++pc;
                uint16_t r = registers[memory[pc]];
                ++pc;
                registers[memory[pc]] = l > r;
                ++pc;
                break;
            };
            // reg < reg -> reg
            case 34: {
                uint16_t l = registers[memory[pc]];
                ++pc;
                uint16_t r = memory[pc];
                ++pc;
                registers[memory[pc]] = l < r;
                ++pc;
                break;
            };
            // reg > reg -> reg
            case 35: {
                uint16_t l = registers[memory[pc]];
                ++pc;
                uint16_t r = registers[memory[pc]];
                ++pc;
                registers[memory[pc]] = l > r;
                ++pc;
                break;
            };
            // TODO: if reg then jump
            default: std::cout << "unknown instruction " << op << std::endl;
        }
    }
}

