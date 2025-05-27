module;

#include <expected>
#include <optional>

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
    explicit Parser(Tokens ts) : ts(ts) {}

    std::expected<ast::Expr, Error> parse();

    std::expected<ast::Expr, Error> parse_stmt();

    std::expected<ast::Expr, Error> parse_expr(int rbp = 0);

    /**
     * @brief null denotation (literals, unary operators)
     */
    std::expected<ast::Expr, Error> parse_expr_nud(Token t);

    /**
     * @brief left denotation (binary operators with infix positions)
     */
    std::expected<ast::Expr, Error> parse_expr_led(Token t, Expr l);

    Token peek();
    std::expected<Token, Error> advance();

    /**
     * @brief matches token type
     * @param tt token type
     * @param x offset
     * @return matches?
     */
    bool match(TokenType tt, size_t x = 0);

    /**
     * @brief matches and consumes token type, fails if cannot match
     * @param tt token type
     * @return void or "expected token" error
     */
    std::expected<Token, Error> expect(TokenType tt);
};

/**
 * @brief builds a Parser and wraps around Parser::parse
 * @see Parser::parse
 */
export inline std::expected<ast::Expr, Error> parse(const Tokens &ts) {
    Parser parser(ts);
    return parser.parse();
}
} // namespace parser
