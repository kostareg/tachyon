#include "parser/parser.hpp"
#include "op.hpp"

namespace parser {
std::expected<ast::Exprs, Error> parse(Tokens) {
    return std::unexpected(Error(ErrorKind::ParseError, "B", 1, 1, 1, 1));
}
} // namespace parser
