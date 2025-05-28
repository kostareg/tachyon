module;

#include <expected>
#include <format>
#include <memory>
#include <string>
#include <unordered_map>
#include <variant>

module parser;

import lexer;
using enum TokenType;

namespace parser {
std::expected<ast::Expr, Error> Parser::parse() {
    std::vector<ast::Expr> stmts;
    while (true) {
        auto stmt = parse_stmt();
        if (!stmt)
            return std::unexpected(stmt.error());

        if (!(match(NLINE) || match(SEMIC)))
            return std::unexpected(Error(ErrorKind::ParseError,
                                         "expected end of statement",
                                         peek().span));

        auto _terminator = advance();

        // get rid of any trailing newlines
        while (match(NLINE)) {
            auto _trail = advance();
        }

        stmts.push_back(std::move(stmt.value()));

        if (match(END))
            break;
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
    else if (t.type == LPAREN) {
        auto e = parse_expr();
        if (!match(RPAREN))
            return std::unexpected(
                Error(ErrorKind::ParseError,
                      "expected one expression in parentheses", peek().span));
        advance();
        return e;
    } else if (t.type == IDENT)
        return Expr(LetRefExpr(std::get<std::string>(t.value)), t.span);
    else if (t.type == FN) {
        if (auto lp = expect(LPAREN); !lp)
            return std::unexpected(lp.error());

        // parse arguments (x, y: Type, z)
        std::unordered_map<std::string, MaybeType> args;
        while (true) {
            auto name = expect(IDENT);
            if (!name)
                return std::unexpected(name.error());
            if (!std::holds_alternative<std::string>(name->value))
                return std::unexpected(Error(
                    ErrorKind::ParseError,
                    "function argument name must be identifer", peek().span));

            MaybeType maybeArgumentType;
            if (match(COLON)) {
                // we have a type
                auto _colon = advance();
                auto argumentType = expect(IDENT);
                if (!argumentType)
                    return std::unexpected(Error(argumentType.error()));
                if (!std::holds_alternative<std::string>(argumentType->value))
                    return std::unexpected(
                        Error(ErrorKind::ParseError,
                              "function argument type must be identifier",
                              peek().span));

                auto namedType = std::get<std::string>(argumentType->value);

                if (namedType == "Num") {
                    maybeArgumentType = BasicConcreteTypes::Number;
                } else if (namedType == "Str") {
                    maybeArgumentType = BasicConcreteTypes::String;
                } else if (namedType == "Bool") {
                    maybeArgumentType = BasicConcreteTypes::Boolean;
                } else if (namedType == "Unit") {
                    maybeArgumentType = BasicConcreteTypes::Unit;
                } else if (namedType == "Fn") {
                    // TODO
                } else {
                    maybeArgumentType = namedType;
                }
            }

            args.emplace(std::get<std::string>(name->value), maybeArgumentType);

            if (match(COMMA)) {
                auto _comma = advance();
            } else {
                break;
            }
        }

        if (auto rp = expect(RPAREN); !rp)
            return std::unexpected(rp.error());

        // parse return type (-> Type)
        MaybeType maybeReturnType;
        if (match(RARROW)) {
            auto _rarrow = advance();
            auto argumentType = expect(IDENT);
            if (!argumentType)
                return std::unexpected(Error(argumentType.error()));
            if (!std::holds_alternative<std::string>(argumentType->value))
                return std::unexpected(Error(
                    ErrorKind::ParseError,
                    "function argument type must be identifier", peek().span));

            auto namedType = std::get<std::string>(argumentType->value);

            if (namedType == "Num") {
                maybeReturnType = BasicConcreteTypes::Number;
            } else if (namedType == "Str") {
                maybeReturnType = BasicConcreteTypes::String;
            } else if (namedType == "Bool") {
                maybeReturnType = BasicConcreteTypes::Boolean;
            } else if (namedType == "Unit") {
                maybeReturnType = BasicConcreteTypes::Unit;
            } else if (namedType == "Fn") {
                // TODO
            } else {
                maybeReturnType = namedType;
            }
        }

        if (auto lb = expect(LBRACE); !lb)
            return std::unexpected(lb.error());

        std::vector<Expr> stmts;
        while (1) {
            auto stmt = parse_stmt();
            if (!stmt)
                return std::unexpected(Error(stmt.error()));

            // TODO
            if (!(match(SEMIC) || match(NLINE))) {
                return std::unexpected(Error(ErrorKind::ParseError,
                                             "expected end of statement",
                                             peek().span));
            }

            auto _terminator = advance();

            // get rid of any trailing newlines
            while (match(NLINE)) {
                auto _trail = advance();
            }

            if (match(END))
                return std::unexpected(Error(
                    ErrorKind::ParseError, "eof during function", peek().span));

            stmts.push_back(std::move(stmt.value()));

            if (match(RBRACE)) {
                auto _rbrace = advance();
                break;
            }
        }

        auto sequence = SequenceExpr(std::move(stmts));

        // finally, we can construct our function type
        return Expr(FnExpr(args, maybeReturnType,
                           std::make_unique<Expr>(std::move(sequence))));
    } else if (t.type == RETURN) {
        auto e = parse_expr();
        if (!e)
            return std::unexpected(e.error());

        return Expr(ReturnExpr(std::make_unique<Expr>(std::move(e.value()))));
    }

    return std::unexpected(
        Error(ErrorKind::ParseError, "failed to parse expression", t.span));
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

bool Parser::match(TokenType tt, size_t x) {
    if (ts[i + x].type == tt) {
        return true;
    }
    return false;
}

std::expected<Token, Error> Parser::expect(TokenType tt) {
    if (match(tt))
        return advance();
    else
        return std::unexpected(Error(
            ErrorKind::ParseError,
            std::format("expected {}", tok_to_str_pretty(tt)), peek().span));
}
} // namespace parser
