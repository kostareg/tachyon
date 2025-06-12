#pragma once

#include "tachyon/runtime/value.h"

#include <array>

namespace tachyon::runtime
{
struct CallFrame
{
    // TODO: maybe optimization point. store large values in their own regs?
    std::array<Value, 256> registers;

    Value returns;
};
} // namespace tachyon::runtime