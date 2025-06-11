module;

#include <expected>
#include <iostream>
#include <string>
#include <variant>
#include <vector>

export module lexer;

import error;
import op;

namespace lexer
{
export enum class TokenType {
    IDENT,
    NUMBER,
    STRING,
    BOOL,
    UNIT,
    EQ,
    PLUS,
    MINUS,
    STAR,
    FSLASH,
    CARET,
    RCHEV, // TODO: rest of comparison
    LPAREN,
    RPAREN,
    LBRACE,
    RBRACE,
    RARROW,
    DOT,
    COLON,
    SEMIC,
    COMMA,
    IMPORT,
    FN,
    RETURN,
    NLINE,
    END
};

using enum TokenType;

export inline std::string tokToStr(TokenType t)
{
    if (t == IDENT)
        return "IDENT";
    else if (t == NUMBER)
        return "NUMBER";
    else if (t == STRING)
        return "STRING";
    else if (t == BOOL)
        return "BOOL";
    else if (t == UNIT)
        return "UNIT";
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
    else if (t == RCHEV)
        return "RCHEV";
    else if (t == CARET)
        return "CARET";
    else if (t == LPAREN)
        return "LPAREN";
    else if (t == RPAREN)
        return "RPAREN";
    else if (t == LBRACE)
        return "LBRACE";
    else if (t == RBRACE)
        return "RBRACE";
    else if (t == RARROW)
        return "RARROW";
    else if (t == DOT)
        return "DOT";
    else if (t == COLON)
        return "COLON";
    else if (t == SEMIC)
        return "SEMIC";
    else if (t == COMMA)
        return "COMMA";
    else if (t == IMPORT)
        return "IMPORT";
    else if (t == FN)
        return "FN";
    else if (t == RETURN)
        return "RETURN";
    else if (t == NLINE)
        return "NLINE";
    else if (t == END)
        return "END";
    else
        return "unknown token";
}

export inline std::string tokToStrPretty(TokenType t)
{
    if (t == IDENT)
        return "an identifier";
    else if (t == NUMBER)
        return "a number";
    else if (t == STRING)
        return "a string";
    else if (t == BOOL)
        return "a boolean";
    else if (t == UNIT)
        return "a unit";
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
    else if (t == RCHEV)
        return "a right chevron";
    else if (t == CARET)
        return "a caret symbol";
    else if (t == LPAREN)
        return "a left parenthesis";
    else if (t == RPAREN)
        return "a right parenthesis";
    else if (t == LBRACE)
        return "a left brace";
    else if (t == RBRACE)
        return "a right brace";
    else if (t == RARROW)
        return "a right arrow";
    else if (t == DOT)
        return "a dot";
    else if (t == COLON)
        return "a colon";
    else if (t == SEMIC)
        return "a semicolon";
    else if (t == COMMA)
        return "a comma";
    else if (t == IMPORT)
        return "an import";
    else if (t == FN)
        return "a function declaration";
    else if (t == RETURN)
        return "a return";
    else if (t == NLINE)
        return "a new line";
    else if (t == END)
        return "the end of the code";
    else
        return "an unknown token";
}

export inline bool isOperator(TokenType t)
{
    return t == PLUS || t == MINUS || t == STAR || t == FSLASH || t == CARET;
};

export inline Op tokToOp(TokenType t)
{
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

/**
 * @brief get left binding power
 *
 * Represents the precedence of a token.
 */
export inline int getLbp(TokenType t)
{
    switch (t)
    {
    case PLUS:
    case MINUS:
        return 10;
    case STAR:
    case FSLASH:
        return 20;
    case CARET:
        return 30;
    default:
        return 0;
    }
}

export struct Token
{
    TokenType type;
    std::variant<std::monostate, double, bool, std::string> value;
    SourceSpan span;

    Token(TokenType type, size_t pos, size_t line, size_t col, size_t len)
        : type(type), span(pos, line, col, len)
    {
    }
    Token(TokenType type, size_t pos, size_t line, size_t col, size_t len, double value)
        : type(type), value(value), span(pos, line, col, len)
    {
    }
    Token(TokenType type, size_t pos, size_t line, size_t col, size_t len, bool value)
        : type(type), value(value), span(pos, line, col, len)
    {
    }
    Token(TokenType type, size_t pos, size_t line, size_t col, size_t len, std::string value)
        : type(type), value(value), span(pos, line, col, len)
    {
    }

    void print()
    {
        std::cout << tokToStr(type);
        if (std::holds_alternative<double>(value))
            std::cout << " " << std::get<double>(value);
        if (std::holds_alternative<bool>(value))
            std::cout << " " << (std::get<bool>(value) ? "TRUE" : "FALSE");
        if (std::holds_alternative<std::string>(value))
            std::cout << " " << std::get<std::string>(value);
    };
};

export using Tokens = std::vector<Token>;

export struct LexerMeta
{
    size_t line = 1;
    size_t col = 1;

    void nline()
    {
        ++line;
        col = 0;
    }
};

export std::expected<Tokens, Error> lex(const std::string &);
} // namespace lexer
