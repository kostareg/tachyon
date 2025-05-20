#include <expected>

#include "ast/ast.hpp"

namespace ast {
std::expected<vm::Proto, Error> generate_proto(ExprRefs) {
    return std::unexpected(Error(ErrorKind::ParseError, "A", 0, 0, 0, 0));
}
} // namespace ast
