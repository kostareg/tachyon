#include "parser/parser.hpp"
#include "op.hpp"

namespace parser {
std::expected<ast::Expr, Error> Parser::parse() {
    while (i < ts.size()) {
        Token curr = ts[i];

        // literals
        if (curr.type == NUMBER)
            return Expr(LiteralExpr(LiteralValue(std::get<double>(curr.value))),
                        curr.span);
        else if (curr.type == STRING)
            return Expr(
                LiteralExpr(LiteralValue(std::get<std::string>(curr.value))),
                curr.span);
        else if (curr.type == BOOL)
            return Expr(LiteralExpr(LiteralValue(std::get<bool>(curr.value))),
                        curr.span);
        else if (curr.type == UNIT)
            return Expr(LiteralExpr(LiteralValue()), curr.span);

        ++i;
    }

    return std::unexpected(Error(ErrorKind::ParseError, "B", 1, 1, 1, 1));
}

std::expected<Token, Error> Parser::next() {
    if (i + 1 < ts.size()) {
        return ts[i + 1];
    } else {
        // return span info from last known token
        return std::unexpected(
            Error(ErrorKind::ParseError, "could not find token", ts[i].span));
    }
}
} // namespace parser
