#pragma once

#include "ast/ast.hpp"
#include "lexer/lexer.hpp"

using namespace ast;
using namespace lexer;

namespace parser {
class Parser {
    /// index of the current token
    size_t i = 0;
    Tokens ts;

  public:
    Parser(Tokens ts) : ts(ts) {}

    std::expected<ast::Expr, Error> parse();

    std::expected<Token, Error> next();
};

/**
 * @brief builds a Parser and wraps around Parser::parse
 * @see Parser::parse
 */
inline std::expected<ast::Expr, Error> parse(Tokens ts) {
    Parser parser(ts);
    return parser.parse();
}
} // namespace parser
