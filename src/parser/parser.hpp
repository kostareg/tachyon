#pragma once

#include "ast/ast.hpp"
#include "lexer/lexer.hpp"

using namespace ast;
using namespace lexer;

namespace parser {

class Parser {
    std::vector<Token> tokens;
    int pos = 0;

    std::unique_ptr<ASTNode> statement();
    std::unique_ptr<ASTNode> expr();
    std::unique_ptr<ASTNode> term();
    std::unique_ptr<ASTNode> factor();

    /// See the nth token.
    Token &peek(int n = 0) { return tokens[pos + n]; };

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
    void expect(TokenType type) {
        if (peek().type != type) {
            printTokenType(type);
            std::cout << std::endl << std::flush;
            throw std::runtime_error("unexpected token. wanted ^");
        }

        advance();
    }

  public:
    Parser(std::vector<Token> tokens) : tokens(tokens) {}

    std::unique_ptr<ASTNode> parse();
};

} // namespace parser
