module;

#include <expected>
#include <memory>
#include <optional>
#include <string>
#include <variant>

module parser;

import lexer;
using enum TokenType;

namespace parser {
std::expected<ast::Expr, Error> Parser::parse() {
    std::vector<ast::Expr> stmts;
    while (auto x = parse_stmt()) {
        if (!x)
            return std::unexpected(x.error());
        if (!match(NLINE))
            return std::unexpected(
                Error(ErrorKind::ParseError, "expected new line", peek().span));
        stmts.push_back(std::move(x.value()));
    }

    return Expr(SequenceExpr(std::move(stmts)));
}

std::expected<ast::Expr, Error> Parser::parse_stmt() {
    if (match(IDENT) && match(EQ, 1)) {
        auto l = advance();
        if (!l)
            return std::unexpected(l.error());
        if (!std::holds_alternative<std::string>(l->value)) {
            return std::unexpected(Error(ErrorKind::ParseError,
                                         "variable name should be text ",
                                         l->span));
        }

        auto _eq = advance();

        auto r = parse_expr();
        if (!r)
            return std::unexpected(r.error());

        return Expr(LetExpr(std::get<std::string>(l->value),
                            std::make_unique<Expr>(std::move(r.value()))),
                    l->span);
    }

    return parse_expr();
}

std::expected<ast::Expr, Error> Parser::parse_expr(int rbp) {
    auto t = advance();
    if (!t)
        return std::unexpected(t.error());

    auto l = parse_expr_nud(t.value());
    if (!l)
        return std::unexpected(l.error());

    while (true) {
        auto next = peek();
        if (rbp >= get_lbp(next.type))
            break;

        t = advance();
        if (!t)
            return std::unexpected(t.error());

        l = parse_expr_led(t.value(), std::move(l.value()));
        if (!l)
            return std::unexpected(l.error());
    }

    return l;
}

std::expected<ast::Expr, Error> Parser::parse_expr_nud(Token t) {
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
    else if (t.type == IDENT)
        return Expr(LetRefExpr(std::get<std::string>(t.value)), t.span);

    return std::unexpected(
        Error(ErrorKind::ParseError, "could not parse expression", t.span));
}

std::expected<ast::Expr, Error> Parser::parse_expr_led(Token t, Expr l) {
    auto r = parse_expr(get_lbp(t.type));
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

bool Parser::match(TokenType tt) {
    if (peek().type == tt) {
        return true;
    }
    return false;
}

bool Parser::match(TokenType tt, size_t x) {
    if (ts[i + x].type == tt) {
        return true;
    }
    return false;
}
} // namespace parser
