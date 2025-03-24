#pragma once

#include <iostream>
#include <optional>
#include <string>
#include <vector>

namespace parser {
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
    END
};

using enum TokenType;

void printTokenType(TokenType t);

inline bool isoperator(TokenType t) {
    return t == PLUS || t == MINUS || t == STAR || t == FSLASH || t == CARET;
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

class Tokenizer {
  public:
    std::vector<Token> tokenize(std::string s);
};
} // namespace parser
