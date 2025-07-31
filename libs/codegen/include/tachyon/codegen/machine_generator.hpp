#pragma once

#include "tachyon/codegen/ir_generator.hpp"
#include "tachyon/common/error.hpp"
#include "tachyon/runtime/proto.hpp"

#include <expected>

namespace tachyon::codegen {
std::expected<void, Error> generate_machine(IRArena, const std::shared_ptr<runtime::Proto> &fn,
                                            runtime::Value *registers);
}