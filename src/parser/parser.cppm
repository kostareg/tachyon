module;

#include <expected>

export module parser;
import ast;
import lexer;
import error;

using namespace ast;
using namespace lexer;

namespace parser {
export class Parser {
    /// index of the current token
    size_t i = 0;
    Tokens ts;

  public:
    Parser(Tokens ts) : ts(ts) {}

    std::expected<ast::Expr, Error> parse(int rbp = 0);

    /**
     * @brief null denotation (literals, unary operators)
     */
    std::expected<ast::Expr, Error> parse_nud(Token t);

    /**
     * @brief left denotation (binary operators with infix positions)
     */
    std::expected<ast::Expr, Error> parse_led(Token t, Expr l);

    Token peek();
    std::expected<Token, Error> advance();
};

/**
 * @brief builds a Parser and wraps around Parser::parse
 * @see Parser::parse
 */
export inline std::expected<ast::Expr, Error> parse(Tokens ts) {
    Parser parser(ts);
    return parser.parse();
}
} // namespace parser
