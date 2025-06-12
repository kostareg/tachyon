#include "tachyon/lexer/lexer.h"

#include <expected>

namespace tachyon::lexer
{
std::expected<Tokens, Error> lex(const std::string &s)
{
    std::vector<Token> tokens;
    std::vector<Error> errors;
    size_t pos = 0;

    while (1)
    {
        if (pos == s.size())
        {
            tokens.emplace_back(END, pos, 0);
            break;
        }

        auto c = s[pos];

        if (c == ' ')
        {
        }
        else if (c == '\n')
        {
            tokens.emplace_back(NLINE, pos, 1);
        }
        else if (c == '/' && s[pos + 1] == '*')
        {
            size_t start_pos = pos;
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
            }
        }
        else if (c == '/' && s[pos + 1] == '/')
        {
            pos += 2;
            while (s[pos] != '\n')
            {
                ++pos;
            }
        }
        else if (c == '=')
            tokens.emplace_back(EQ, pos, 1);
        else if (c == '+')
            tokens.emplace_back(PLUS, pos, 1);
        else if (c == '-')
        {
            // could be ->
            if (s[pos + 1] == '>')
            {
                tokens.emplace_back(RARROW, pos, 2);
                ++pos;
            }
            else
            {
                tokens.emplace_back(MINUS, pos, 1);
            }
        }
        else if (c == '*')
            tokens.emplace_back(STAR, pos, 1);
        else if (c == '/')
            tokens.emplace_back(FSLASH, pos, 1);
        else if (c == '>')
            tokens.emplace_back(RCHEV, pos, 1);
        else if (c == '^')
            tokens.emplace_back(CARET, pos, 1);
        else if (c == '(')
        {
            // may be unit type ()
            if (s[pos + 1] == ')')
            {
                tokens.emplace_back(UNIT, pos, 2);
                ++pos;
            }
            else
                tokens.emplace_back(LPAREN, pos, 1);
        }
        else if (c == ')')
            tokens.emplace_back(RPAREN, pos, 1);
        else if (c == '{')
            tokens.emplace_back(LBRACE, pos, 1);
        else if (c == '}')
            tokens.emplace_back(RBRACE, pos, 1);
        else if (c == '.')
            tokens.emplace_back(DOT, pos, 1);
        else if (c == ':')
            tokens.emplace_back(COLON, pos, 1);
        else if (c == ';')
            tokens.emplace_back(SEMIC, pos, 1);
        else if (c == ',')
            tokens.emplace_back(COMMA, pos, 1);
        else if (c == '"')
        {
            std::string content;
            int start_pos = pos;
            ++pos;
            while (s[pos] != '"')
            {
                content += s[pos];
                ++pos;
            }
            tokens.emplace_back(STRING, start_pos, pos + 1 - start_pos, content);
        }
        else if (s.substr(pos, 4) == "True" && !isalpha(s[pos + 4]))
        {
            // ^- `True a` -> bool ident, `Truea` -> ident.
            tokens.emplace_back(BOOL, pos, 4, true);
            pos += 3;
        }
        else if (s.substr(pos, 5) == "False" && !isalpha(s[pos + 5]))
        {
            tokens.emplace_back(BOOL, pos, 5, false);
            pos += 4;
        }
        else if (s.substr(pos, 6) == "import" && !isalpha(s[pos + 6]))
        {
            tokens.emplace_back(IMPORT, pos, 6);
            pos += 5;
        }
        else if (s.substr(pos, 2) == "fn" && !isalpha(s[pos + 2]))
        {
            tokens.emplace_back(FN, pos, 2);
            pos += 1;
        }
        else if (s.substr(pos, 6) == "return" && !isalpha(s[pos + 6]))
        {
            tokens.emplace_back(RETURN, pos, 6);
            pos += 5;
        }
        else if (isalpha(c))
        {
            // idents must start with alpha, nums and ' accepted after.
            std::string i;
            size_t start_pos = pos;
            while (isalpha(s[pos]) || isdigit(s[pos]) || s[pos] == '\'')
            {
                i += s[pos];
                ++pos;
            }
            auto len = pos - start_pos;
            tokens.emplace_back(IDENT, start_pos, len, i);
            --pos;
        }
        else if (isdigit(c))
        {
            std::string n;
            size_t start_pos = pos;
            while ((isdigit(s[pos]) && !isspace(s[pos])) || s[pos] == '.')
            {
                n += s[pos];
                ++pos;
            }
            auto len = pos - start_pos;
            tokens.emplace_back(NUMBER, start_pos, len, std::stod(n));
            --pos;
        }
        else
            errors.push_back(
                Error::create(ErrorKind::LexError, SourceSpan(pos, 1), "unknown character")
                    .withCode("E0001"));

        ++pos;
    }

    if (!errors.empty())
    {
        return std::unexpected(Error::createMultiple(errors));
    }

    // log
    // for (auto tok : tokens)
    // {
    // tok.print();
    // std::printf("\n");
    // }
    // std::cout << std::endl;

    return tokens;
}
} // namespace tachyon::lexer