#pragma once

#include "tachyon/runtime/value.h"

#include "tachyon/common/error.h"
#include "tachyon/runtime/call_frame.h"
#include "tachyon/runtime/proto.h"

#include <cstdint>
#include <expected>
#include <vector>

namespace tachyon::runtime
{
class VM
{
    std::vector<CallFrame> call_stack;

  public:
    VM()
    {
        auto initial_frame = CallFrame{};
        call_stack.push_back(std::move(initial_frame));
    }

    std::expected<void, Error> run(const Proto &proto);
    std::expected<void, Error> call(std::shared_ptr<Proto> fn, uint8_t offset);
    void doctor() const;
};
} // namespace tachyon::runtime
