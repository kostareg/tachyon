#pragma once

#include "../../../../parser/include/tachyon/parser/op.h"
#include "tachyon/common/source_span.h"
#include "tachyon/lexer/token.h"

#include <iostream>
#include <stdexcept>
#include <variant>

namespace tachyon::lexer
{
/**
 * @brief type of token that can be found in a Tachyon source file
 */
enum class TokenType
{
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
    RCHEV,
    // TODO: rest of the comparison operators eg >=, ==, !=, etc.
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

/**
 * @brief find token as string
 * @param tt token type
 * @return stringified
 */
inline std::string tokToStr(TokenType tt)
{
    if (tt == IDENT)
        return "IDENT";
    else if (tt == NUMBER)
        return "NUMBER";
    else if (tt == STRING)
        return "STRING";
    else if (tt == BOOL)
        return "BOOL";
    else if (tt == UNIT)
        return "UNIT";
    else if (tt == EQ)
        return "EQ";
    else if (tt == PLUS)
        return "PLUS";
    else if (tt == MINUS)
        return "MINUS";
    else if (tt == STAR)
        return "STAR";
    else if (tt == FSLASH)
        return "FSLASH";
    else if (tt == RCHEV)
        return "RCHEV";
    else if (tt == CARET)
        return "CARET";
    else if (tt == LPAREN)
        return "LPAREN";
    else if (tt == RPAREN)
        return "RPAREN";
    else if (tt == LBRACE)
        return "LBRACE";
    else if (tt == RBRACE)
        return "RBRACE";
    else if (tt == RARROW)
        return "RARROW";
    else if (tt == DOT)
        return "DOT";
    else if (tt == COLON)
        return "COLON";
    else if (tt == SEMIC)
        return "SEMIC";
    else if (tt == COMMA)
        return "COMMA";
    else if (tt == IMPORT)
        return "IMPORT";
    else if (tt == FN)
        return "FN";
    else if (tt == RETURN)
        return "RETURN";
    else if (tt == NLINE)
        return "NLINE";
    else if (tt == END)
        return "END";
    else
        return "unrecognized token";
}

/**
 * @brief find token as pretty string, for user-end
 * @param tt token type
 * @return pretty stringified
 */
inline std::string tokToStrPretty(TokenType tt)
{
    if (tt == IDENT)
        return "an identifier";
    else if (tt == NUMBER)
        return "a number";
    else if (tt == STRING)
        return "a string";
    else if (tt == BOOL)
        return "a boolean";
    else if (tt == UNIT)
        return "a unit";
    else if (tt == EQ)
        return "an equals sign";
    else if (tt == PLUS)
        return "a plus sign";
    else if (tt == MINUS)
        return "a minus sign";
    else if (tt == STAR)
        return "a star";
    else if (tt == FSLASH)
        return "a slash";
    else if (tt == RCHEV)
        return "a right chevron";
    else if (tt == CARET)
        return "a caret symbol";
    else if (tt == LPAREN)
        return "a left parenthesis";
    else if (tt == RPAREN)
        return "a right parenthesis";
    else if (tt == LBRACE)
        return "a left brace";
    else if (tt == RBRACE)
        return "a right brace";
    else if (tt == RARROW)
        return "a right arrow";
    else if (tt == DOT)
        return "a dot";
    else if (tt == COLON)
        return "a colon";
    else if (tt == SEMIC)
        return "a semicolon";
    else if (tt == COMMA)
        return "a comma";
    else if (tt == IMPORT)
        return "an import";
    else if (tt == FN)
        return "a function declaration";
    else if (tt == RETURN)
        return "a return";
    else if (tt == NLINE)
        return "a new line";
    else if (tt == END)
        return "the end of the code";
    else
        return "an unrecognized token";
}

/**
 * @brief checks if a token is an operator (one of +, -, *, /, ^)
 * @param tt token type
 * @return is operator?
 */
inline bool isOperator(TokenType tt)
{
    return tt == PLUS || tt == MINUS || tt == STAR || tt == FSLASH || tt == CARET;
};

/**
 * @brief turn an operator token into an `Op`
 * @param tt token type
 * @return operator
 * @see tachyon::parser::Op
 */
inline parser::Op tokToOp(TokenType tt)
{
    if (tt == PLUS)
        return parser::Op::Add;
    else if (tt == MINUS)
        return parser::Op::Sub;
    else if (tt == STAR)
        return parser::Op::Mul;
    else if (tt == FSLASH)
        return parser::Op::Div;
    else if (tt == CARET)
        return parser::Op::Pow;
    else
        throw std::runtime_error("ice: unknown operator");
};

/**
 * @brief get left binding power
 *
 * Represents the precedence of a token.
 *
 * @param tt token type
 */
inline int getLbp(TokenType tt)
{
    switch (tt)
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

/**
 * @brief represents one typed, valued, and spanned Tachyon token
 */
struct Token
{
    /// token type
    TokenType type;

    /// value, defaults to std::monostate
    std::variant<std::monostate, double, bool, std::string> value;

    /// relevant source region
    SourceSpan span;

    Token(TokenType type, size_t pos, size_t len) : type(type), span(pos, len) {}
    Token(TokenType type, size_t pos, size_t len, double value)
        : type(type), value(value), span(pos, len)
    {
    }
    Token(TokenType type, size_t pos, size_t len, bool value)
        : type(type), value(value), span(pos, len)
    {
    }
    Token(TokenType type, size_t pos, size_t len, std::string value)
        : type(type), value(value), span(pos, len)
    {
    }

    /**
     * @brief print token
     */
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

/// list of tokens
using Tokens = std::vector<Token>;
} // namespace tachyon::lexer