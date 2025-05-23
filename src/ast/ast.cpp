module;

#include <expected>

module ast;

namespace ast {
std::expected<vm::Proto, Error> generate_proto(Expr) {
    return std::unexpected(Error(ErrorKind::ParseError, "A", 0, 0, 0, 0));
}
} // namespace ast
