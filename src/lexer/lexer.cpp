#include <fmt/format.h>
#include <iostream>

#include "lexer/lexer.hpp"

namespace lexer {
using enum TokenType;

void printTokenType(TokenType t) {
    if (t == IDENT)
        std::cout << "IDENT";
    else if (t == NUMBER)
        std::cout << "NUMBER";
    else if (t == EQ)
        std::cout << "EQ";
    else if (t == PLUS)
        std::cout << "PLUS";
    else if (t == MINUS)
        std::cout << "MINUS";
    else if (t == STAR)
        std::cout << "STAR";
    else if (t == FSLASH)
        std::cout << "FSLASH";
    else if (t == CARET)
        std::cout << "CARET";
    else if (t == LPAREN)
        std::cout << "LPAREN";
    else if (t == RPAREN)
        std::cout << "RPAREN";
    else if (t == SEMIC)
        std::cout << "SEMIC";
    else if (t == COMMA)
        std::cout << "COMMA";
    else if (t == END)
        std::cout << "END";
    else
        std::cout << "unknown token";
}

Result<std::vector<Token>> Lexer::lex(const std::string &s) {
    std::vector<Token> tokens;
    size_t pos = 0;
    LexerMetadata m;

    while (1) {
        if (pos == s.size()) {
            tokens.push_back(Token(END));
            break;
        }

        auto c = s[pos];

        if (c == ' ') {
        } else if (c == '\n')
            m.nline();
        else if (c == '/' && s[pos + 1] == '*') {
            size_t startPos = pos;
            size_t startLine = m.line;
            size_t startCol = m.col;
            pos += 3;
            while (s[pos - 1] != '*' && s[pos] != '/') {
                ++pos;
                // EOF
                if (pos > s.size()) {
                    return Error(ErrorKind::LexError, "comment left open", get_line_at(s, startPos),
                                 startLine, startCol, 2)
                        .with_code("E0002")
                        .with_hint("complete the comment with `*/`.");
                }
                if (s[pos] == '\n')
                    m.nline();
            }
        } else if (c == '/' && s[pos + 1] == '/') {
            pos += 2;
            while (s[pos] != '\n') {
                ++pos;
            }
            m.nline();
        } else if (c == '=')
            tokens.push_back(EQ);
        else if (c == '+')
            tokens.push_back(PLUS);
        else if (c == '-')
            tokens.push_back(MINUS);
        else if (c == '*')
            tokens.push_back(STAR);
        else if (c == '/')
            tokens.push_back(FSLASH);
        else if (c == '^')
            tokens.push_back(CARET);
        else if (c == '(')
            tokens.push_back(LPAREN);
        else if (c == ')')
            tokens.push_back(RPAREN);
        else if (c == ';')
            tokens.push_back(SEMIC);
        else if (c == ',')
            tokens.push_back(COMMA);
        else if (isalpha(c)) {
            std::string i;
            while (isalpha(s[pos]) && !isspace(s[pos])) {
                i += s[pos];
                ++pos;
            }
            --pos;
            tokens.push_back(Token(IDENT, i));
        } else if (isdigit(c)) {
            std::string n;
            while (isdigit(s[pos]) && !isspace(s[pos])) {
                n += s[pos];
                ++pos;
            }
            --pos;
            tokens.push_back(Token(NUMBER, stoi(n)));
        } else
            return Error(ErrorKind::LexError, "unknown character", get_line_at(s, pos), m.line,
                         m.col, 1)
                .with_code("E0001");

        ++pos;
        ++m.col;
    }

    // ...

    return tokens;
}

} // namespace lexer
