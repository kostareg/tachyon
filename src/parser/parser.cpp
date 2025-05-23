#include "parser/parser.hpp"

namespace parser {
std::expected<ast::Expr, Error> Parser::parse(int rbp) {
    auto t = advance();
    if (!t)
        return std::unexpected(t.error());

    auto l = parse_nud(t.value());
    if (!l)
        return std::unexpected(l.error());

    while (true) {
        auto next = peek();
        if (rbp >= get_lbp(next.type))
            break;

        t = advance();
        if (!t)
            return std::unexpected(t.error());

        l = parse_led(t.value(), std::move(l.value()));
        if (!l)
            return std::unexpected(l.error());
    }

    return l;
}

std::expected<ast::Expr, Error> Parser::parse_nud(Token t) {
    if (t.type == NUMBER)
        return Expr(LiteralExpr(LiteralValue(std::get<double>(t.value))),
                    t.span);
    else if (t.type == STRING)
        return Expr(LiteralExpr(LiteralValue(std::get<std::string>(t.value))),
                    t.span);
    else if (t.type == BOOL)
        return Expr(LiteralExpr(LiteralValue(std::get<bool>(t.value))), t.span);
    else if (t.type == UNIT)
        return Expr(LiteralExpr(LiteralValue()), t.span);

    return std::unexpected(
        Error(ErrorKind::ParseError, "could not find literal", t.span));
}

std::expected<ast::Expr, Error> Parser::parse_led(Token t, Expr l) {
    auto r = parse(get_lbp(t.type));
    if (!r)
        return std::unexpected(r.error());
    return Expr(BinaryOperatorExpr(
        tok_to_op(t.type), std::make_unique<Expr>(std::move(l)),
        std::make_unique<Expr>(std::move(r.value()))));
}

Token Parser::peek() { return ts[i]; }

std::expected<Token, Error> Parser::advance() {
    if (i + 1 >= ts.size())
        return std::unexpected(Error(ErrorKind::ParseError, "", ts[i].span));
    return ts[i++];
}
} // namespace parser
