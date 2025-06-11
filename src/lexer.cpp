#include <expected>
#include <iostream>
#include <string>

#include "lexer.h"

namespace lexer
{
std::expected<Tokens, Error> lex(const std::string &s)
{
    std::vector<Token> tokens;
    std::vector<Error> errors;
    size_t pos = 0;
    LexerMeta m;

    while (1)
    {
        if (pos == s.size())
        {
            tokens.emplace_back(END, pos, m.line, m.col, 0);
            break;
        }

        auto c = s[pos];

        if (c == ' ')
        {
        }
        else if (c == '\n')
        {
            tokens.emplace_back(NLINE, pos, m.line, m.col, 1);
            m.nline();
        }
        else if (c == '/' && s[pos + 1] == '*')
        {
            size_t start_pos = pos;
            size_t start_line = m.line;
            size_t start_col = m.col;
            pos += 3;
            while (!(s[pos - 1] == '*' && s[pos] == '/'))
            {
                ++pos;
                // EOF
                if (pos >= s.size())
                {
                    return std::unexpected(Error::create(ErrorKind::LexError,
                                                         SourceSpan(start_pos, 2),
                                                         "comment left open")
                                               .withCode("E0002")
                                               .withHint("complete the comment with `*/`."));
                }
                if (s[pos] == '\n')
                    m.nline();
            }
        }
        else if (c == '/' && s[pos + 1] == '/')
        {
            pos += 2;
            while (s[pos] != '\n')
            {
                ++pos;
            }
            m.nline();
        }
        else if (c == '=')
            tokens.emplace_back(EQ, pos, m.line, m.col, 1);
        else if (c == '+')
            tokens.emplace_back(PLUS, pos, m.line, m.col, 1);
        else if (c == '-')
        {
            // could be ->
            if (s[pos + 1] == '>')
            {
                tokens.emplace_back(RARROW, pos, m.line, m.col, 2);
                ++pos, ++m.line;
            }
            else
            {
                tokens.emplace_back(MINUS, pos, m.line, m.col, 1);
            }
        }
        else if (c == '*')
            tokens.emplace_back(STAR, pos, m.line, m.col, 1);
        else if (c == '/')
            tokens.emplace_back(FSLASH, pos, m.line, m.col, 1);
        else if (c == '>')
            tokens.emplace_back(RCHEV, pos, m.line, m.col, 1);
        else if (c == '^')
            tokens.emplace_back(CARET, pos, m.line, m.col, 1);
        else if (c == '(')
        {
            // may be unit type ()
            if (s[pos + 1] == ')')
            {
                tokens.emplace_back(UNIT, pos, m.line, m.col, 2);
                ++pos;
                ++m.col;
            }
            else
                tokens.emplace_back(LPAREN, pos, m.line, m.col, 1);
        }
        else if (c == ')')
            tokens.emplace_back(RPAREN, pos, m.line, m.col, 1);
        else if (c == '{')
            tokens.emplace_back(LBRACE, pos, m.line, m.col, 1);
        else if (c == '}')
            tokens.emplace_back(RBRACE, pos, m.line, m.col, 1);
        else if (c == '.')
            tokens.emplace_back(DOT, pos, m.line, m.col, 1);
        else if (c == ':')
            tokens.emplace_back(COLON, pos, m.line, m.col, 1);
        else if (c == ';')
            tokens.emplace_back(SEMIC, pos, m.line, m.col, 1);
        else if (c == ',')
            tokens.emplace_back(COMMA, pos, m.line, m.col, 1);
        else if (c == '"')
        {
            std::string content;
            int start_pos = pos;
            int start_line = m.line;
            int start_col = m.col;
            ++pos;
            ++m.col;
            while (s[pos] != '"')
            {
                content += s[pos];
                if (s[pos] == '\n')
                    m.nline();
                ++pos;
                ++m.col;
            }
            tokens.emplace_back(STRING, start_pos, start_line, start_col, pos + 1 - start_pos,
                                content);
        }
        else if (s.substr(pos, 4) == "True" && !isalpha(s[pos + 4]))
        {
            // ^- `True a` -> bool ident, `Truea` -> ident.
            tokens.emplace_back(BOOL, pos, m.line, m.col, 4, true);
            pos += 3;
            m.col += 3;
        }
        else if (s.substr(pos, 5) == "False" && !isalpha(s[pos + 5]))
        {
            tokens.emplace_back(BOOL, pos, m.line, m.col, 5, false);
            pos += 4;
            m.col += 4;
        }
        else if (s.substr(pos, 6) == "import" && !isalpha(s[pos + 6]))
        {
            tokens.emplace_back(IMPORT, pos, m.line, m.col, 6);
            pos += 5;
            m.col += 5;
        }
        else if (s.substr(pos, 2) == "fn" && !isalpha(s[pos + 2]))
        {
            tokens.emplace_back(FN, pos, m.line, m.col, 2);
            pos += 1;
            m.col += 1;
        }
        else if (s.substr(pos, 6) == "return" && !isalpha(s[pos + 6]))
        {
            tokens.emplace_back(RETURN, pos, m.line, m.col, 6);
            pos += 5;
            m.col += 5;
        }
        else if (isalpha(c))
        {
            // idents must start with alpha, nums and ' accepted after.
            std::string i;
            size_t start_pos = pos;
            size_t start_line = m.line;
            size_t start_col = m.col;
            while (isalpha(s[pos]) || isdigit(s[pos]) || s[pos] == '\'')
            {
                i += s[pos];
                ++pos;
            }
            auto len = pos - start_pos;
            tokens.emplace_back(IDENT, start_pos, start_line, start_col, len, i);
            m.col += len - 1;
            --pos;
        }
        else if (isdigit(c))
        {
            std::string n;
            size_t start_pos = pos;
            size_t start_line = m.line;
            size_t start_col = m.col;
            while ((isdigit(s[pos]) && !isspace(s[pos])) || s[pos] == '.')
            {
                n += s[pos];
                ++pos;
            }
            auto len = pos - start_pos;
            tokens.emplace_back(NUMBER, start_pos, start_line, start_col, len, std::stod(n));
            m.col += len - 1;
            --pos;
        }
        else
            errors.push_back(
                Error::create(ErrorKind::LexError, SourceSpan(pos, 1), "unknown character")
                    .withCode("E0001"));

        ++pos;
        ++m.col;
    }

    if (!errors.empty())
    {
        return std::unexpected(Error::createMultiple(errors));
    }

    // log
    for (auto tok : tokens)
    {
        tok.print();
        std::printf("\n");
    }
    std::cout << std::endl;

    return tokens;
}
} // namespace lexer
