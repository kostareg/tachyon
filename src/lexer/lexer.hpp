#pragma once

#include <iostream>
#include <optional>
#include <string>
#include <vector>

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
    END
};

using enum TokenType;

void printTokenType(TokenType t);

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

    Token(TokenType type) : type(type) {}
    Token(TokenType type, int value) : type(type), value(value) {}
    Token(TokenType type, std::string ident) : type(type), ident(ident) {}

    void print() {
        printTokenType(type);
        if (value)
            std::cout << " " << *value;
        if (!ident.empty())
            std::cout << " " << ident;
    };
};

class Lexer {
  public:
    std::vector<Token> lex(std::string s);
};
} // namespace lexer
