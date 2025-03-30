#pragma once

#include "ast/ast.hpp"
#include "lexer/lexer.hpp"

using namespace ast;
using namespace lexer;

namespace parser {

class Parser {
    std::vector<Token> tokens;
    int pos = 0;

    Result<std::unique_ptr<ASTNode>> statement();
    Result<std::unique_ptr<ASTNode>> expr();

    /// See the nth token.
    Token &peek(size_t n = 0) {
        if (pos + n >= tokens.size())
            return tokens.back(); // TODO: Result + error
        return tokens[pos + n];
    };

    /// Consume the next token.
    Token &advance() { return tokens[pos++]; };

    /// Consume if token type matches. Silent fail.
    bool match(TokenType type) {
        if (peek().type == type) {
            advance();
            return true;
        }
        return false;
    };

    /// Consume if token type matches, returning the whole token. Silent fail.
    std::optional<Token> matchRead(TokenType type) {
        if (peek().type == type) {
            return advance();
        }
        return std::nullopt;
    }

    /// Consume token type. Parser error if fails.
    Result<void> expect(TokenType type) {
        auto next = peek();
        if (next.type != type) {
            return Error(ErrorKind::ParseError, "unexpected token", next.m.pos, next.m.line,
                         next.m.col, next.m.len)
                .with_code("E0003")
                .with_hint("was looking for " + tok_to_str_pretty(type) + ", found " +
                           tok_to_str_pretty(next.type) + ".");
        }

        advance();
        return Result<void>{};
    }

  public:
    Parser(std::vector<Token> tokens) : tokens(tokens) {}

    Result<std::unique_ptr<ASTNode>> parse();
};

} // namespace parser
