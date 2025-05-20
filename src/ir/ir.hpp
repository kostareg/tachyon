#pragma once

#include <memory>
#include <string>
#include <vector>

#include "error.hpp"
#include "op.hpp"

/**
 * @namespace ir
 * @brief High-level intermediate representation nodes.
 */
namespace ir {
using ID = uint32_t;

enum class Op {
    LoadVar,
    StoreVar,
};

struct Instr {
    Op op;
    std::vector<Op> args;
    ID result;
};
} // namespace ir
