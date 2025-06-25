#pragma once

#include "tachyon/common/op.h"
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
    ECOMP,
    NECOMP,
    LCOMP,
    GCOMP,
    LECOMP,
    GECOMP,
    NOT,
    BAND,
    BOR,
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
    WHILE,
    BREAK,
    CONTINUE,
    NLINE,
    END
};

using enum TokenType;

// TODO: hashmap for these? or better way?
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
    else if (tt == CARET)
        return "CARET";
    else if (tt == ECOMP)
        return "ECOMP";
    else if (tt == NECOMP)
        return "NECOMP";
    else if (tt == LCOMP)
        return "LCOMP";
    else if (tt == GCOMP)
        return "GCOMP";
    else if (tt == LECOMP)
        return "LECOMP";
    else if (tt == GECOMP)
        return "GECOMP";
    else if (tt == NOT)
        return "NOT";
    else if (tt == BAND)
        return "BAND";
    else if (tt == BOR)
        return "BOR";
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
    else if (tt == WHILE)
        return "WHILE";
    else if (tt == BREAK)
        return "BREAK";
    else if (tt == CONTINUE)
        return "CONTINUE";
    else if (tt == NLINE)
        return "NLINE";
    else if (tt == END)
        return "END";
    else
        return "unrecognized token";
}

/**
 * @brief find string of token
 * @param str token string
 * @return token
 */
inline TokenType strToTok(std::string_view str)
{
    if (str == "IDENT")
        return IDENT;
    else if (str == "NUMBER")
        return NUMBER;
    else if (str == "STRING")
        return STRING;
    else if (str == "BOOL")
        return BOOL;
    else if (str == "UNIT")
        return UNIT;
    else if (str == "EQ")
        return EQ;
    else if (str == "PLUS")
        return PLUS;
    else if (str == "MINUS")
        return MINUS;
    else if (str == "STAR")
        return STAR;
    else if (str == "FSLASH")
        return FSLASH;
    else if (str == "CARET")
        return CARET;
    else if (str == "ECOMP")
        return ECOMP;
    else if (str == "NECOMP")
        return NECOMP;
    else if (str == "LCOMP")
        return LCOMP;
    else if (str == "GCOMP")
        return GCOMP;
    else if (str == "LECOMP")
        return LECOMP;
    else if (str == "GECOMP")
        return GECOMP;
    else if (str == "NOT")
        return NOT;
    else if (str == "BAND")
        return BAND;
    else if (str == "BOR")
        return BOR;
    else if (str == "LPAREN")
        return LPAREN;
    else if (str == "RPAREN")
        return RPAREN;
    else if (str == "LBRACE")
        return LBRACE;
    else if (str == "RBRACE")
        return RBRACE;
    else if (str == "RARROW")
        return RARROW;
    else if (str == "DOT")
        return DOT;
    else if (str == "COLON")
        return COLON;
    else if (str == "SEMIC")
        return SEMIC;
    else if (str == "COMMA")
        return COMMA;
    else if (str == "IMPORT")
        return IMPORT;
    else if (str == "FN")
        return FN;
    else if (str == "RETURN")
        return RETURN;
    else if (str == "WHILE")
        return WHILE;
    else if (str == "BREAK")
        return BREAK;
    else if (str == "CONTINUE")
        return CONTINUE;
    else if (str == "NLINE")
        return NLINE;
    else if (str == "END")
        return END;
    else
        throw std::runtime_error(std::format("failed to turn str into token type: {}", str));
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
    else if (tt == CARET)
        return "a caret symbol";
    else if (tt == ECOMP)
        return "an equality comparison";
    else if (tt == NECOMP)
        return "an inequality comparison";
    else if (tt == LCOMP)
        return "a less than sign";
    else if (tt == GCOMP)
        return "a greater than sign";
    else if (tt == LECOMP)
        return "a less than or equal to sign";
    else if (tt == GECOMP)
        return "a greater than or equal to sign";
    else if (tt == NOT)
        return "a not operator";
    else if (tt == BAND)
        return "a boolean and operator";
    else if (tt == BOR)
        return "a boolean or operator";
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
    else if (tt == WHILE)
        return "a while loop";
    else if (tt == BREAK)
        return "a break";
    else if (tt == CONTINUE)
        return "a continue";
    else if (tt == NLINE)
        return "a new line";
    else if (tt == END)
        return "the end of the code";
    else
        return "an unrecognized token";
}

/**
 * @brief checks if a token is an operator
 * @param tt token type
 * @return is operator?
 */
inline bool isOperator(TokenType tt)
{
    return tt == ECOMP || tt == NECOMP || tt == LCOMP || tt == GCOMP || tt == LECOMP ||
           tt == GECOMP || tt == PLUS || tt == MINUS || tt == STAR || tt == FSLASH || tt == CARET ||
           tt == NOT || tt == BAND || tt == BOR;
};

/**
 * @brief turn an operator token into an `Op`
 * @param tt token type
 * @return operator
 * @see tachyon::parser::Op
 */
inline parser::Op tokToOp(TokenType tt)
{
    if (tt == ECOMP)
        return parser::Op::Eq;
    else if (tt == NECOMP)
        return parser::Op::Neq;
    else if (tt == LCOMP)
        return parser::Op::Lst;
    else if (tt == GCOMP)
        return parser::Op::Grt;
    else if (tt == LECOMP)
        return parser::Op::Lset;
    else if (tt == GECOMP)
        return parser::Op::Gret;
    else if (tt == NOT)
        return parser::Op::Not;
    else if (tt == BAND)
        return parser::Op::And;
    else if (tt == BOR)
        return parser::Op::Or;
    else if (tt == PLUS)
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
    case ECOMP:
    case NECOMP:
        return 40;
    case LCOMP:
    case GCOMP:
    case LECOMP:
    case GECOMP:
        return 50;
    case BAND:
    case BOR:
        return 60;
    case NOT:
        return 70;
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