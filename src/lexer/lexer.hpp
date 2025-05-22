#pragma once

#include <expected>
#include <iostream>
#include <optional>
#include <string>
#include <vector>

#include "error.hpp"
#include "op.hpp"

namespace lexer {
enum class TokenType {
    IDENT,
    NUMBER,
    EQ,
    PLUS,
    MINUS,
    STAR,
    FSLASH,
    CARET,
    LPAREN,
    RPAREN,
    SEMIC,
    COMMA,
    NLINE,
    END
};

using enum TokenType;

inline std::string tok_to_str(TokenType t) {
    if (t == IDENT)
        return "IDENT";
    else if (t == NUMBER)
        return "NUMBER";
    else if (t == EQ)
        return "EQ";
    else if (t == PLUS)
        return "PLUS";
    else if (t == MINUS)
        return "MINUS";
    else if (t == STAR)
        return "STAR";
    else if (t == FSLASH)
        return "FSLASH";
    else if (t == CARET)
        return "CARET";
    else if (t == LPAREN)
        return "LPAREN";
    else if (t == RPAREN)
        return "RPAREN";
    else if (t == SEMIC)
        return "SEMIC";
    else if (t == COMMA)
        return "COMMA";
    else if (t == NLINE)
        return "NLINE";
    else if (t == END)
        return "END";
    else
        return "unknown token";
}

inline std::string tok_to_str_pretty(TokenType t) {
    if (t == IDENT)
        return "an identifier";
    else if (t == NUMBER)
        return "a number";
    else if (t == EQ)
        return "an equals sign";
    else if (t == PLUS)
        return "a plus sign";
    else if (t == MINUS)
        return "a minus sign";
    else if (t == STAR)
        return "a star";
    else if (t == FSLASH)
        return "a slash";
    else if (t == CARET)
        return "a caret symbol";
    else if (t == LPAREN)
        return "a left parenthesis";
    else if (t == RPAREN)
        return "a right parenthesis";
    else if (t == SEMIC)
        return "a semicolon";
    else if (t == COMMA)
        return "a comma";
    else if (t == NLINE)
        return "a new line";
    else if (t == END)
        return "the end of the code";
    else
        return "an unknown token";
}

inline bool isoperator(TokenType t) {
    return t == PLUS || t == MINUS || t == STAR || t == FSLASH || t == CARET;
};

inline Op tok_to_op(TokenType t) {
    if (t == PLUS)
        return Op::Add;
    else if (t == MINUS)
        return Op::Sub;
    else if (t == STAR)
        return Op::Mul;
    else if (t == FSLASH)
        return Op::Div;
    else if (t == CARET)
        return Op::Pow;
    else
        throw std::runtime_error("ice: unknown operator");
};

struct Token {
    TokenType type;
    std::optional<int> value;
    std::string ident;
    SourceSpan span;

    Token(TokenType type, size_t pos, size_t line, size_t col, size_t len)
        : type(type), span(pos, line, col, len) {}
    Token(TokenType type, size_t pos, size_t line, size_t col, size_t len,
          int value)
        : type(type), value(value), span(pos, line, col, len) {}
    Token(TokenType type, size_t pos, size_t line, size_t col, size_t len,
          std::string ident)
        : type(type), ident(ident), span(pos, line, col, len) {}

    void print() {
        std::cout << tok_to_str(type);
        if (value)
            std::cout << " " << *value;
        if (!ident.empty())
            std::cout << " " << ident;
    };
};

using Tokens = std::vector<Token>;

struct LexerMeta {
    size_t line = 1;
    size_t col = 1;

    void nline() {
        ++line;
        col = 0;
    }
};

std::expected<Tokens, Error> lex(const std::string);
} // namespace lexer
