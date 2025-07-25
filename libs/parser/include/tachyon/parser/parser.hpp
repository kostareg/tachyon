#pragma once

#include "tachyon/lexer/lexer.hpp"
#include "tachyon/parser/ast.hpp"

#include <expected>
#include <utility>

namespace tachyon::parser {
/**
 * @brief generates abstract syntax tree from list of tokens
 */
class Parser {
    /**
     * @brief token index
     */
    size_t i = 0;

    /**
     * @brief token parse list
     */
    lexer::Tokens ts;

    std::vector<lexer::Value> lexer_constants;

    /**
     * @brief left denotation (binary operators with infix positions)
     */
    std::expected<Expr, Error> parse_expr_led(lexer::Token t, Expr l);

    /**
     * @brief null denotation (literals, unary operators)
     */
    std::expected<Expr, Error> parse_expr_nud(lexer::Token t);

    /**
     * @brief parse next expression
     * @param rbp right binding power
     * @return expression
     */
    std::expected<Expr, Error> parse_expr(int rbp = 0);

    /**
     * @brief parse next statement
     * @return statement
     */
    std::expected<Expr, Error> parse_stmt();

    /**
     * @brief peek current token
     * @return current token
     */
    lexer::Token peek();

    /**
     * @brief consume current token
     * @return last token
     */
    std::expected<lexer::Token, Error> advance();

    /**
     * @brief matches token type
     * @param tt token type
     * @param x offset
     * @return matches?
     */
    bool match(lexer::TokenType tt, size_t x = 0);

    /**
     * @brief matches and consumes token type, fails if cannot match
     * @param tt token type
     * @return void or "expected token" error
     */
    std::expected<lexer::Token, Error> expect(lexer::TokenType tt);

  public:
    Parser(lexer::Tokens ts, std::vector<lexer::Value> cs)
        : ts(std::move(ts)), lexer_constants(std::move(cs)) {}

    std::expected<Expr, Error> parse();
};

/**
 * @brief builds a Parser given tokens, and runs Parser::parse
 * @param ts tokens
 * @see Parser::parse
 */
inline std::expected<Expr, Error> parse(lexer::Tokens ts, std::vector<lexer::Value> cs) {
    Parser parser(std::move(ts), std::move(cs));
    return parser.parse();
}
} // namespace tachyon::parser
