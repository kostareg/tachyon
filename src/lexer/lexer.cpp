#include <fmt/format.h>
#include <spdlog/spdlog.h>

#include "lexer/lexer.hpp"

namespace lexer {
std::expected<Tokens, Error> lex(const std::string s) {
    std::vector<Token> tokens;
    std::vector<Error> errors;
    size_t pos = 0;
    LexerMeta m;

    while (1) {
        if (pos == s.size()) {
            tokens.push_back(Token(END, pos, m.line, m.col, 0));
            break;
        }

        auto c = s[pos];

        if (c == ' ') {
        } else if (c == '\n') {
            tokens.emplace_back(NLINE, pos, m.line, m.col, 1);
            m.nline();
        } else if (c == '/' && s[pos + 1] == '*') {
            size_t startPos = pos;
            size_t startLine = m.line;
            size_t startCol = m.col;
            pos += 3;
            while (!(s[pos - 1] == '*' && s[pos] == '/')) {
                ++pos;
                // EOF
                if (pos >= s.size()) {
                    return std::unexpected(
                        Error(ErrorKind::LexError, "comment left open",
                              startPos, startLine, startCol, 2)
                            .with_code("E0002")
                            .with_hint("complete the comment with `*/`."));
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
            tokens.emplace_back(EQ, pos, m.line, m.col, 1);
        else if (c == '+')
            tokens.emplace_back(PLUS, pos, m.line, m.col, 1);
        else if (c == '-')
            tokens.emplace_back(MINUS, pos, m.line, m.col, 1);
        else if (c == '*')
            tokens.emplace_back(STAR, pos, m.line, m.col, 1);
        else if (c == '/')
            tokens.emplace_back(FSLASH, pos, m.line, m.col, 1);
        else if (c == '^')
            tokens.emplace_back(CARET, pos, m.line, m.col, 1);
        else if (c == '(')
            tokens.emplace_back(LPAREN, pos, m.line, m.col, 1);
        else if (c == ')')
            tokens.emplace_back(RPAREN, pos, m.line, m.col, 1);
        else if (c == ';')
            tokens.emplace_back(SEMIC, pos, m.line, m.col, 1);
        else if (c == ',')
            tokens.emplace_back(COMMA, pos, m.line, m.col, 1);
        else if (isalpha(c)) {
            std::string i;
            size_t startPos = pos;
            size_t startLine = m.line;
            size_t startCol = m.col;
            while (isalpha(s[pos]) &&
                   !isspace(s[pos])) { // TODO: remove isspace
                i += s[pos];
                ++pos;
            }
            auto len = pos - startPos;
            tokens.emplace_back(IDENT, startPos, startLine, startCol, len, i);
            m.col += len - 1;
            --pos;
        } else if (isdigit(c)) {
            std::string n;
            size_t startPos = pos;
            size_t startLine = m.line;
            size_t startCol = m.col;
            while (isdigit(s[pos]) && !isspace(s[pos])) {
                n += s[pos];
                ++pos;
            }
            auto len = pos - startPos;
            tokens.emplace_back(NUMBER, startPos, startLine, startCol, len,
                                stoi(n));
            m.col += len - 1;
            --pos;
        } else
            errors.push_back(Error(ErrorKind::LexError, "unknown character",
                                   pos, m.line, m.col, 1)
                                 .with_code("E0001"));

        ++pos;
        ++m.col;
    }

    if (!errors.empty()) {
        return std::unexpected(Error(errors));
    }

    // log
    for (auto tok : tokens) {
        std::cout << tok_to_str(tok.type) << std::endl;
    }
    std::cout << std::endl;

    return tokens;
}
} // namespace lexer
