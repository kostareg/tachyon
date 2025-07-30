#pragma once

#include "tachyon/common/error.hpp"
#include "tachyon/runtime/proto.hpp"

#include <expected>

namespace tachyon::codegen {
    std::expected<void, Error> generate_machine(std::shared_ptr<runtime::Proto>);
}