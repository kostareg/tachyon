#include "tachyon/parser/parser.hpp"

#include "tachyon/common/error.hpp"
#include "tachyon/common/log.hpp"
#include "tachyon/lexer/token.hpp"
#include "tachyon/parser/ast.hpp"
#include "tachyon/parser/print.hpp"

#include <expected>
#include <format>
#include <memory>
#include <string>
#include <unordered_map>
#include <variant>

using enum tachyon::lexer::TokenType;

using namespace tachyon::lexer;

namespace tachyon::parser {
std::expected<Expr, Error> Parser::parse_expr_led(Token t, Expr l) {
    auto r = parse_expr(get_lbp(t.type));
    if (!r) return std::unexpected(r.error());
    return Expr(BinaryOperatorExpr(tok_to_op(t.type), std::make_unique<Expr>(std::move(l)),
                                   std::make_unique<Expr>(std::move(r.value()))));
}

std::expected<Expr, Error> Parser::parse_expr_nud(Token t) {
    if (t.type == NUMBER)
        return Expr(LiteralExpr(LiteralValue(std::get<double>(lexer_constants[t.constant_ptr]))));
    else if (t.type == STRING)
        return Expr(
            LiteralExpr(LiteralValue(std::get<std::string>(lexer_constants[t.constant_ptr]))));
    else if (t.type == TRUE) return Expr(LiteralExpr(LiteralValue(true)));
    else if (t.type == FALSE) return Expr(LiteralExpr(LiteralValue(false)));
    else if (t.type == UNIT) return Expr(LiteralExpr(LiteralValue()));
    else if (t.type == NOT) {
        auto e = parse_expr();
        if (!e) return std::unexpected(e.error());
        return Expr(UnaryOperatorExpr(Op::Not, std::make_unique<Expr>(std::move(e.value()))),
                    SourceSpan(0, 0));
    } else if (t.type == LPAREN) {
        auto e = parse_expr();
        if (!match(RPAREN))
            return std::unexpected(Error::create(ErrorKind::ParseError, SourceSpan(0, 0),
                                                 "expected one expression in parentheses"));
        auto _rparen = advance();
        return e;
    } else if (t.type == LBRACK) {
        if (match(RBRACK)) {
            auto _rbrack = advance();
            return Expr(LiteralExpr(LiteralValue(Matrix())));
        }
        std::vector<Expr> list;
        size_t height = 1;
        while (!match(RBRACK)) {
            while (match(NLINE))
                auto _nline = advance();

            auto e = parse_expr();
            if (!e) {
                return std::unexpected(e.error());
            }
            list.push_back(std::move(e.value()));

            if (match(RBRACK)) {
                auto _rbrack = advance();
                break;
            }
            if (match(SEMIC)) {
                auto _semic = advance();
                ++height;
                continue;
            }

            while (match(NLINE))
                auto _nline = advance();

            if (auto comma = expect(COMMA); !comma) {
                return std::unexpected(comma.error());
            }

            while (match(NLINE))
                auto _nline = advance();
        }
        return Expr(MatrixConstructExpr(height, std::move(list)));
    } else if (t.type == IDENT) {
        auto value = std::get<std::string>(lexer_constants[t.constant_ptr]);
        auto ref = LetRefExpr(std::get<std::string>(lexer_constants[t.constant_ptr]));

        if (match(UNIT)) {
            // we are working with a function call, no args
            auto _parens = advance();
            return Expr(FnCallExpr(std::move(ref), {}));
        } else if (match(LPAREN) && match(RPAREN, 1)) {
            // we are working with a function call, no args
            auto _lparen = advance();
            auto _rparen = advance();
            return Expr(FnCallExpr(std::move(ref), {}));
        } else if (match(LPAREN)) {
            // we are working with a function call, with args
            auto _lparen = advance();

            std::vector<Expr> args;

            while (1) {
                auto e = parse_expr();
                if (!e) return std::unexpected(e.error());
                args.push_back(std::move(e.value()));

                if (match(RPAREN)) break;

                if (auto comma = expect(COMMA); !comma) return std::unexpected(comma.error());
            }

            if (auto rparen = expect(RPAREN); !rparen) return std::unexpected(rparen.error());

            return Expr(FnCallExpr(std::move(ref), std::move(args)));
        } else if (match(LBRACK)) {
            // we are working with a matrix reference
            auto _lbrack = advance();
            auto row = parse_expr();
            if (!row) return std::unexpected(row.error());
            auto comma = expect(COMMA);
            if (!comma) return std::unexpected(comma.error());
            auto col = parse_expr();
            if (!col) return std::unexpected(col.error());
            auto rbrack = expect(RBRACK);
            if (!rbrack) return std::unexpected(rbrack.error());
            return Expr(MatrixRefExpr(ref, std::make_unique<Expr>(std::move(row.value())),
                                      std::make_unique<Expr>(std::move(col.value()))));
        } else return Expr(ref);
    } else if (t.type == WHILE) {
        auto condition = parse_expr();
        if (!condition) return std::unexpected(condition.error());

        if (auto lb = expect(LBRACE); !lb) return std::unexpected(lb.error());

        // TODO: this is repeated in function and parse, can we move it into one helper function?
        std::vector<Expr> stmts;
        while (1) {
            // get rid of any leading newlines
            while (match(NLINE)) {
                auto _leading = advance();
            }

            auto stmt = parse_stmt();
            if (!stmt) return std::unexpected(stmt.error());

            // TODO
            if (!(match(SEMIC) || match(NLINE))) {
                return std::unexpected(Error::create(ErrorKind::ParseError, SourceSpan(0, 0),
                                                     "expected end of statement"));
            }

            auto _terminator = advance();

            // get rid of any trailing newlines
            while (match(NLINE)) {
                auto _trail = advance();
            }

            if (match(END))
                return std::unexpected(
                    Error::create(ErrorKind::ParseError, SourceSpan(0, 0), "eof during function"));

            stmts.push_back(std::move(stmt.value()));

            if (match(RBRACE)) {
                auto _rbrace = advance();
                break;
            }
        }

        auto body = SequenceExpr(std::move(stmts));

        return Expr(WhileLoopExpr(std::make_unique<Expr>(std::move(condition).value()),
                                  std::make_unique<Expr>(std::move(body))),
                    SourceSpan(0, 0));
    } else if (t.type == IF) {
        auto condition = parse_expr();
        if (!condition) return std::unexpected(condition.error());
        if (auto lb = expect(LBRACE); !lb) return std::unexpected(lb.error());

        // TODO: this is repeated in function and parse, can we move it into one helper function?
        std::vector<Expr> stmts;
        while (1) {
            // get rid of any leading newlines
            while (match(NLINE)) {
                auto _leading = advance();
            }

            auto stmt = parse_stmt();
            if (!stmt) return std::unexpected(stmt.error());

            // TODO
            if (!(match(SEMIC) || match(NLINE))) {
                return std::unexpected(Error::create(ErrorKind::ParseError, SourceSpan(0, 0),
                                                     "expected end of statement"));
            }

            auto _terminator = advance();

            // get rid of any trailing newlines
            while (match(NLINE)) {
                auto _trail = advance();
            }

            if (match(END))
                return std::unexpected(
                    Error::create(ErrorKind::ParseError, SourceSpan(0, 0), "eof during function"));

            stmts.push_back(std::move(stmt.value()));

            if (match(RBRACE)) {
                auto _rbrace = advance();
                break;
            }
        }

        auto body = SequenceExpr(std::move(stmts));

        Exprs else_if_conditions = {};
        Exprs else_if_bodies = {};
        ExprRef else_body = nullptr;

        while (1) {
            if (match(ELSE)) {
                auto _else = advance();
                if (match(IF)) {
                    auto _if = advance();
                    auto condition = parse_expr();
                    if (!condition) return std::unexpected(condition.error());
                    if (auto lb = expect(LBRACE); !lb) return std::unexpected(lb.error());

                    // TODO: this is repeated in function and parse, can we move it into one helper
                    // function?
                    std::vector<Expr> stmts;
                    while (1) {
                        // get rid of any leading newlines
                        while (match(NLINE)) {
                            auto _leading = advance();
                        }

                        auto stmt = parse_stmt();
                        if (!stmt) return std::unexpected(stmt.error());

                        // TODO
                        if (!(match(SEMIC) || match(NLINE))) {
                            return std::unexpected(Error::create(ErrorKind::ParseError,
                                                                 SourceSpan(0, 0),
                                                                 "expected end of statement"));
                        }

                        auto _terminator = advance();

                        // get rid of any trailing newlines
                        while (match(NLINE)) {
                            auto _trail = advance();
                        }

                        if (match(END))
                            return std::unexpected(Error::create(
                                ErrorKind::ParseError, SourceSpan(0, 0), "eof during function"));

                        stmts.push_back(std::move(stmt.value()));

                        if (match(RBRACE)) {
                            auto _rbrace = advance();
                            break;
                        }
                    }

                    else_if_conditions.emplace_back(std::move(*condition));
                    else_if_bodies.emplace_back(SequenceExpr(std::move(stmts)));
                } else {
                    if (auto lb = expect(LBRACE); !lb) return std::unexpected(lb.error());

                    // TODO: this is repeated in function and parse, can we move it into one helper
                    //  function?
                    std::vector<Expr> stmts;
                    while (1) {
                        // get rid of any leading newlines
                        while (match(NLINE)) {
                            auto _leading = advance();
                        }

                        auto stmt = parse_stmt();
                        if (!stmt) return std::unexpected(stmt.error());

                        // TODO
                        if (!(match(SEMIC) || match(NLINE))) {
                            return std::unexpected(Error::create(ErrorKind::ParseError,
                                                                 SourceSpan(0, 0),
                                                                 "expected end of statement"));
                        }

                        auto _terminator = advance();

                        // get rid of any trailing newlines
                        while (match(NLINE)) {
                            auto _trail = advance();
                        }

                        if (match(END))
                            return std::unexpected(Error::create(
                                ErrorKind::ParseError, SourceSpan(0, 0), "eof during function"));

                        stmts.push_back(std::move(stmt.value()));

                        if (match(RBRACE)) {
                            auto _rbrace = advance();
                            break;
                        }
                    }

                    else_body = std::make_unique<Expr>(SequenceExpr(std::move(stmts)));

                    break;
                }
            } else {
                break;
            }
        }

        TY_ASSERT(else_if_conditions.size() == else_if_bodies.size());

        return Expr(IfExpr(std::make_unique<Expr>(std::move(*condition)),
                           std::make_unique<Expr>(std::move(body)), std::move(else_if_conditions),
                           std::move(else_if_bodies), std::move(else_body)));
    } else if (t.type == FN) {
        if (auto lp = expect(LPAREN); !lp) return std::unexpected(lp.error());

        // parse arguments (x, y: Type, z)
        std::vector<std::pair<std::string, MaybeType>> args;
        while (true) {
            auto name = expect(IDENT);
            if (!name) return std::unexpected(name.error());

            MaybeType maybe_argument_type;
            if (match(COLON)) {
                // we have a type
                auto colon = advance();
                auto argument_type = expect(IDENT);
                if (!argument_type) return std::unexpected(argument_type.error());

                auto named_type =
                    std::get<std::string>(lexer_constants[argument_type->constant_ptr]);

                if (named_type == "Num") maybe_argument_type = BasicConcreteTypes::Number;
                else if (named_type == "Str") maybe_argument_type = BasicConcreteTypes::String;
                else if (named_type == "Bool") maybe_argument_type = BasicConcreteTypes::Boolean;
                else if (named_type == "Unit") maybe_argument_type = BasicConcreteTypes::Unit;
                else if (named_type == "Fn") {
                    // TODO
                } else maybe_argument_type = named_type;
            }

            args.emplace_back(std::get<std::string>(lexer_constants[name->constant_ptr]),
                              maybe_argument_type);

            if (match(COMMA)) {
                auto _comma = advance();
            } else {
                break;
            }
        }

        if (auto rp = expect(RPAREN); !rp) return std::unexpected(rp.error());

        // parse return type (-> Type)
        MaybeType maybe_return_type;
        if (match(RARROW)) {
            auto rarrow = advance();
            auto argument_type = expect(IDENT);
            if (!argument_type) return std::unexpected(argument_type.error());

            auto named_type = std::get<std::string>(lexer_constants[argument_type->constant_ptr]);

            if (named_type == "Num") maybe_return_type = BasicConcreteTypes::Number;
            else if (named_type == "Str") maybe_return_type = BasicConcreteTypes::String;
            else if (named_type == "Bool") maybe_return_type = BasicConcreteTypes::Boolean;
            else if (named_type == "Unit") maybe_return_type = BasicConcreteTypes::Unit;
            else if (named_type == "Fn") {
                // TODO
            } else maybe_return_type = named_type;
        }

        if (auto lb = expect(LBRACE); !lb) return std::unexpected(lb.error());

        // skip any newlines
        while (match(NLINE))
            auto _nline = advance();

        std::vector<Expr> stmts;
        while (1) {
            auto stmt = parse_stmt();
            if (!stmt) return std::unexpected(stmt.error());

            // TODO
            if (!(match(SEMIC) || match(NLINE))) {
                return std::unexpected(Error::create(ErrorKind::ParseError, SourceSpan(0, 0),
                                                     "expected end of statement"));
            }

            auto _terminator = advance();

            // get rid of any trailing newlines
            while (match(NLINE)) {
                auto _trail = advance();
            }

            if (match(END))
                return std::unexpected(
                    Error::create(ErrorKind::ParseError, SourceSpan(0, 0), "eof during function"));

            stmts.push_back(std::move(stmt.value()));

            if (match(RBRACE)) {
                auto _rbrace = advance();
                break;
            }
        }

        auto sequence = SequenceExpr(std::move(stmts));

        // finally, we can construct our function type
        return Expr(FnExpr(args, maybe_return_type, std::make_unique<Expr>(std::move(sequence))));
    } else if (t.type == RETURN) {
        auto e = parse_expr();
        if (!e) return std::unexpected(e.error());

        return Expr(ReturnExpr(std::make_unique<Expr>(std::move(e.value()))));
    }

    return std::unexpected(
        Error::create(ErrorKind::ParseError, SourceSpan(0, 0), "failed to parse expression"));
}

std::expected<Expr, Error> Parser::parse_expr(int rbp) {
    auto t = advance();
    if (!t) return std::unexpected(t.error());

    auto l = parse_expr_nud(t.value());
    if (!l) return std::unexpected(l.error());

    while (true) {
        auto next = peek();
        if (rbp >= get_lbp(next.type)) break;

        t = advance();
        if (!t) return std::unexpected(t.error());

        l = parse_expr_led(t.value(), std::move(l.value()));
        if (!l) return std::unexpected(l.error());
    }

    return l;
}

std::expected<Expr, Error> Parser::parse_stmt() {
    if (match(IDENT) && match(EQ, 1)) {
        // assignment
        auto l = advance();
        if (!l) return std::unexpected(l.error());

        auto _eq = advance();

        auto r = parse_expr();
        if (!r) return std::unexpected(r.error());

        return Expr(LetExpr(std::get<std::string>(lexer_constants[l->constant_ptr]),
                            std::make_unique<Expr>(std::move(r.value()))),
                    SourceSpan(0, 0));
    } else if (match(IDENT) && match(LBRACK, 1)) {
        // matrix assignment
        auto l = advance();
        if (!l) return std::unexpected(l.error());

        auto _lbrack = advance();
        auto row = parse_expr();
        if (!row) return std::unexpected(row.error());
        auto comma = expect(COMMA);
        if (!comma) return std::unexpected(comma.error());
        auto col = parse_expr();
        if (!col) return std::unexpected(col.error());
        auto rbrack = expect(RBRACK);
        if (!rbrack) return std::unexpected(rbrack.error());

        auto eq = expect(EQ);
        if (!eq) return std::unexpected(eq.error());

        auto r = parse_expr();
        if (!r) return std::unexpected(r.error());

        return Expr(MatrixAssignmentExpr(
            LetRefExpr(std::get<std::string>(lexer_constants[l->constant_ptr])),
            std::make_unique<Expr>(std::move(row.value())),
            std::make_unique<Expr>(std::move(col.value())),
            std::make_unique<Expr>(std::move(r.value()))));
    } else if (match(IMPORT)) {
        // import
        auto _import = advance();

        std::string path;
        while (true) {
            if (auto e = peek(); e.type == IDENT)
                path += std::get<std::string>(lexer_constants[e.constant_ptr]);
            else if (e.type == DOT) path += ".";
            else if (e.type == SEMIC || e.type == NLINE) break;
            else
                return std::unexpected(Error::create(ErrorKind::ParseError, SourceSpan(0, 0),
                                                     "import statement cannot read this path"));

            auto _element = advance();
        }

        return Expr(ImportExpr(path));
    }

    // plain expression
    return parse_expr();
}

Token Parser::peek() {
    return ts[i];
}

std::expected<Token, Error> Parser::advance() {
    if (i + 1 >= ts.size())
        return std::unexpected(Error::create(ErrorKind::ParseError, SourceSpan(0, 0), ""));
    return ts[i++];
}

bool Parser::match(TokenType tt, size_t x) {
    if (ts[i + x].type == tt) {
        return true;
    }
    return false;
}

std::expected<Token, Error> Parser::expect(TokenType tt) {
    if (match(tt)) return advance();
    else
        return std::unexpected(Error::create(ErrorKind::ParseError, SourceSpan(0, 0),
                                             std::format("expected {}", tok_to_str_pretty(tt))));
}

std::expected<Expr, Error> Parser::parse() {
    // if there are no token, return a blank sequence expression
    if (ts.empty()) return Expr(SequenceExpr());

    std::vector<Expr> stmts;
    while (true) {
        // get rid of any leading newlines
        while (match(NLINE)) {
            auto _leading = advance();
        }

        auto stmt = parse_stmt();
        if (!stmt) return std::unexpected(stmt.error());

        if (!(match(NLINE) || match(SEMIC) || match(END)))
            return std::unexpected(Error::create(ErrorKind::ParseError, SourceSpan(0, 0),
                                                 "expected end of statement"));

        if (match(END)) {
            stmts.push_back(std::move(stmt.value()));
            break;
        }

        auto _terminator = advance();

        // get rid of any trailing newlines
        while (match(NLINE)) {
            auto _trail = advance();
        }

        stmts.push_back(std::move(stmt.value()));

        if (match(END)) break;
    }

    Expr sequence{SequenceExpr(std::move(stmts))};

#ifdef TY_DEBUG
    printExpr(sequence);
#endif

    return std::move(sequence);
}

} // namespace tachyon::parser
