#include "tachyon/codegen/ir_generator.hpp"

#include "tachyon/common/error.hpp"
#include "tachyon/runtime/bytecode.hpp"

#include <expected>

namespace tachyon::codegen {
using enum runtime::Bytecode;
std::expected<IRArena, Error> generate_ir(const std::shared_ptr<runtime::Proto> &fn) {
    IRArena arena{};

    for (size_t i = 0; i < fn->bytecode.size(); i++) {
        switch (fn->bytecode[i]) {
        case RETR: {
            IRPointer lhs = make_register(fn->bytecode[++i]);
            arena.emplace(IRNodeKind::Ret, lhs);
            break;
        }
        case MARR: {
            IRPointer lhs = make_register(fn->bytecode[++i]);
            IRPointer rhs = make_register(fn->bytecode[++i]);
            IRPointer dst = make_register(fn->bytecode[++i]);
            arena.emplace(IRNodeKind::Add, lhs, rhs, dst);
            break;
        }
        case MARC: {
            IRPointer lhs = make_register(fn->bytecode[++i]);
            IRPointer rhs = make_constant(fn->bytecode[++i]);
            IRPointer dst = make_register(fn->bytecode[++i]);
            arena.emplace(IRNodeKind::Add, lhs, rhs, dst);
            break;
        }
        default:
            return std::unexpected(
                Error::create(ErrorKind::IRGenerationError, SourceSpan(0, 0), "could not optimize")
                    .withHint(std::format("Selected bytecode 0x{:0x}.", fn->bytecode[i])));
        }
    }

    return std::move(arena);
}
} // namespace tachyon::codegen