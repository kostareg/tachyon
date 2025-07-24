#pragma once

#include "tachyon/runtime/value.hpp"

#include <array>

namespace tachyon::runtime
{
/**
 * @brief function call frame
 *
 * Stores 256 registers per functon call.
 */
struct CallFrame
{
    // TODO: maybe optimization point. store large values in their own regs?
    std::array<Value, 256> registers;

    Value returns;
};
} // namespace tachyon::runtime