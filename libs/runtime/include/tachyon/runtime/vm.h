#pragma once

#include "tachyon/common/error.h"
#include "tachyon/runtime/call_frame.h"
#include "tachyon/runtime/proto.h"
#include "tachyon/runtime/value.h"

#include <cstdint>
#include <expected>
#include <vector>

namespace tachyon::runtime
{
/**
 * @brief Tachyon virtual machine
 */
class VM
{
    /// call stack for all functions
    std::vector<CallFrame> call_stack;

    /**
     * @brief loads arguments (starting from offset) to new call frame and calls function pointer
     * @param fn function pointer
     * @param offset argument offset
     * @return void or error
     */
    std::expected<void, Error> call(std::shared_ptr<Proto> fn, uint8_t offset);

  public:
    VM()
    {
        CallFrame initial_frame{};
        call_stack.push_back(std::move(initial_frame));
    }

    /**
     * @brief run function
     * @param proto function prototype
     * @return void or error
     */
    std::expected<void, Error> run(const Proto &proto);

    /**
     * @brief print current state
     */
    void doctor() const;
};
} // namespace tachyon::runtime
