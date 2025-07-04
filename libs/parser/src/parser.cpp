#include "tachyon/parser/parser.h"

#include "tachyon/common/error.h"
#include "tachyon/lexer/token.h"
#include "tachyon/parser/ast.h"

#include <expected>
#include <format>
#include <memory>
#include <string>
#include <unordered_map>
#include <variant>

using enum tachyon::lexer::TokenType;

using namespace tachyon::lexer;

namespace tachyon::parser
{
std::expected<Expr, Error> Parser::parse_expr_led(Token t, Expr l)
{
    auto r = parse_expr(getLbp(t.type));
    if (!r)
        return std::unexpected(r.error());
    return Expr(BinaryOperatorExpr(tokToOp(t.type), std::make_unique<Expr>(std::move(l)),
                                   std::make_unique<Expr>(std::move(r.value()))));
}

std::expected<Expr, Error> Parser::parse_expr_nud(Token t)
{
    if (t.type == NUMBER)
        return Expr(LiteralExpr(LiteralValue(std::get<double>(t.value))), t.span);
    else if (t.type == STRING)
        return Expr(LiteralExpr(LiteralValue(std::get<std::string>(t.value))), t.span);
    else if (t.type == BOOL)
        return Expr(LiteralExpr(LiteralValue(std::get<bool>(t.value))), t.span);
    else if (t.type == UNIT)
        return Expr(LiteralExpr(LiteralValue()), t.span);
    else if (t.type == NOT)
    {
        auto e = parse_expr();
        if (!e)
            return std::unexpected(e.error());
        return Expr(UnaryOperatorExpr(Op::Not, std::make_unique<Expr>(std::move(e.value()))),
                    t.span);
    }
    else if (t.type == LPAREN)
    {
        auto e = parse_expr();
        if (!match(RPAREN))
            return std::unexpected(Error::create(ErrorKind::ParseError, peek().span,
                                                 "expected one expression in parentheses"));
        auto _rparen = advance();
        return e;
    }
    else if (t.type == IDENT)
    {
        auto value = std::get<std::string>(t.value);
        auto ref = LetRefExpr(std::get<std::string>(t.value));

        if (match(UNIT))
        {
            // we are working with a function call, no args
            auto _parens = advance();
            return Expr(FnCallExpr(std::move(ref), {}));
        }
        else if (match(LPAREN) && match(RPAREN, 1))
        {
            // we are working with a function call, no args
            auto _lparen = advance();
            auto _rparen = advance();
            return Expr(FnCallExpr(std::move(ref), {}));
        }
        else if (match(LPAREN))
        {
            // we are working with a function call, with args
            auto _lparen = advance();

            std::vector<Expr> args;

            while (1)
            {
                auto e = parse_expr();
                if (!e)
                    return std::unexpected(e.error());
                args.push_back(std::move(e.value()));

                if (match(RPAREN))
                    break;

                if (auto comma = expect(COMMA); !comma)
                    return std::unexpected(comma.error());
            }

            if (auto rparen = expect(RPAREN); !rparen)
                return std::unexpected(rparen.error());

            return Expr(FnCallExpr(std::move(ref), std::move(args)), t.span);
        }
        else
            return Expr(ref, t.span);
    }
    else if (t.type == WHILE)
    {
        auto condition = parse_expr();
        if (!condition)
            return std::unexpected(condition.error());

        if (auto lb = expect(LBRACE); !lb)
            return std::unexpected(lb.error());

        // TODO: this is repeated in function and parse, can we move it into one helper function?
        std::vector<Expr> stmts;
        while (1)
        {
            // get rid of any leading newlines
            while (match(NLINE))
            {
                auto _leading = advance();
            }

            auto stmt = parse_stmt();
            if (!stmt)
                return std::unexpected(stmt.error());

            // TODO
            if (!(match(SEMIC) || match(NLINE)))
            {
                return std::unexpected(Error::create(ErrorKind::ParseError,
                                                     SourceSpan(peek().span.position, 1),
                                                     "expected end of statement"));
            }

            auto _terminator = advance();

            // get rid of any trailing newlines
            while (match(NLINE))
            {
                auto _trail = advance();
            }

            if (match(END))
                return std::unexpected(
                    Error::create(ErrorKind::ParseError, peek().span, "eof during function"));

            stmts.push_back(std::move(stmt.value()));

            if (match(RBRACE))
            {
                auto _rbrace = advance();
                break;
            }
        }

        auto body = SequenceExpr(std::move(stmts));

        return Expr(WhileLoopExpr(std::make_unique<Expr>(std::move(condition).value()),
                                  std::make_unique<Expr>(std::move(body))),
                    t.span);
    }
    else if (t.type == FN)
    {
        if (auto lp = expect(LPAREN); !lp)
            return std::unexpected(lp.error());

        // parse arguments (x, y: Type, z)
        std::vector<std::pair<std::string, MaybeType>> args;
        while (true)
        {
            auto name = expect(IDENT);
            if (!name)
                return std::unexpected(name.error());

            MaybeType maybeArgumentType;
            if (match(COLON))
            {
                // we have a type
                auto _colon = advance();
                auto argumentType = expect(IDENT);
                if (!argumentType)
                    return std::unexpected(argumentType.error());

                auto namedType = std::get<std::string>(argumentType->value);

                if (namedType == "Num")
                    maybeArgumentType = BasicConcreteTypes::Number;
                else if (namedType == "Str")
                    maybeArgumentType = BasicConcreteTypes::String;
                else if (namedType == "Bool")
                    maybeArgumentType = BasicConcreteTypes::Boolean;
                else if (namedType == "Unit")
                    maybeArgumentType = BasicConcreteTypes::Unit;
                else if (namedType == "Fn")
                {
                    // TODO
                }
                else
                    maybeArgumentType = namedType;
            }

            args.emplace_back(std::get<std::string>(name->value), maybeArgumentType);

            if (match(COMMA))
            {
                auto _comma = advance();
            }
            else
            {
                break;
            }
        }

        if (auto rp = expect(RPAREN); !rp)
            return std::unexpected(rp.error());

        // parse return type (-> Type)
        MaybeType maybeReturnType;
        if (match(RARROW))
        {
            auto _rarrow = advance();
            auto argumentType = expect(IDENT);
            if (!argumentType)
                return std::unexpected(argumentType.error());

            auto namedType = std::get<std::string>(argumentType->value);

            if (namedType == "Num")
                maybeReturnType = BasicConcreteTypes::Number;
            else if (namedType == "Str")
                maybeReturnType = BasicConcreteTypes::String;
            else if (namedType == "Bool")
                maybeReturnType = BasicConcreteTypes::Boolean;
            else if (namedType == "Unit")
                maybeReturnType = BasicConcreteTypes::Unit;
            else if (namedType == "Fn")
            {
                // TODO
            }
            else
                maybeReturnType = namedType;
        }

        if (auto lb = expect(LBRACE); !lb)
            return std::unexpected(lb.error());

        std::vector<Expr> stmts;
        while (1)
        {
            auto stmt = parse_stmt();
            if (!stmt)
                return std::unexpected(stmt.error());

            // TODO
            if (!(match(SEMIC) || match(NLINE)))
            {
                return std::unexpected(Error::create(ErrorKind::ParseError,
                                                     SourceSpan(peek().span.position, 1),
                                                     "expected end of statement"));
            }

            auto _terminator = advance();

            // get rid of any trailing newlines
            while (match(NLINE))
            {
                auto _trail = advance();
            }

            if (match(END))
                return std::unexpected(
                    Error::create(ErrorKind::ParseError, peek().span, "eof during function"));

            stmts.push_back(std::move(stmt.value()));

            if (match(RBRACE))
            {
                auto _rbrace = advance();
                break;
            }
        }

        auto sequence = SequenceExpr(std::move(stmts));

        // finally, we can construct our function type
        return Expr(FnExpr(args, maybeReturnType, std::make_unique<Expr>(std::move(sequence))));
    }
    else if (t.type == RETURN)
    {
        auto e = parse_expr();
        if (!e)
            return std::unexpected(e.error());

        return Expr(ReturnExpr(std::make_unique<Expr>(std::move(e.value()))));
    }

    return std::unexpected(
        Error::create(ErrorKind::ParseError, t.span, "failed to parse expression"));
}

std::expected<Expr, Error> Parser::parse_expr(int rbp)
{
    auto t = advance();
    if (!t)
        return std::unexpected(t.error());

    auto l = parse_expr_nud(t.value());
    if (!l)
        return std::unexpected(l.error());

    while (true)
    {
        auto next = peek();
        if (rbp >= getLbp(next.type))
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

std::expected<Expr, Error> Parser::parse_stmt()
{
    if (match(IDENT) && match(EQ, 1))
    {
        auto l = advance();
        if (!l)
            return std::unexpected(l.error());

        auto _eq = advance();

        auto r = parse_expr();
        if (!r)
            return std::unexpected(r.error());

        return Expr(
            LetExpr(std::get<std::string>(l->value), std::make_unique<Expr>(std::move(r.value()))),
            l->span);
    }
    else if (match(IMPORT))
    {
        auto _import = advance();

        std::string path;
        while (true)
        {
            if (auto e = peek(); e.type == IDENT)
                path += std::get<std::string>(e.value);
            else if (e.type == DOT)
                path += ".";
            else if (e.type == SEMIC || e.type == NLINE)
                break;
            else
                return std::unexpected(Error::create(ErrorKind::ParseError, e.span,
                                                     "import statement cannot read this path"));

            auto _element = advance();
        }

        return Expr(ImportExpr(path));
    }

    return parse_expr();
}

Token Parser::peek()
{
    return ts[i];
}

std::expected<Token, Error> Parser::advance()
{
    if (i + 1 >= ts.size())
        return std::unexpected(Error::create(ErrorKind::ParseError, ts[i].span, ""));
    return ts[i++];
}

bool Parser::match(TokenType tt, size_t x)
{
    if (ts[i + x].type == tt)
    {
        return true;
    }
    return false;
}

std::expected<Token, Error> Parser::expect(TokenType tt)
{
    if (match(tt))
        return advance();
    else
        return std::unexpected(Error::create(ErrorKind::ParseError, peek().span,
                                             std::format("expected {}", tokToStrPretty(tt))));
}

std::expected<Expr, Error> Parser::parse()
{
    // if there are no token, return a blank sequence expression
    if (ts.empty())
        return Expr(SequenceExpr());

    std::vector<Expr> stmts;
    while (true)
    {
        // get rid of any leading newlines
        while (match(NLINE))
        {
            auto _leading = advance();
        }

        auto stmt = parse_stmt();
        if (!stmt)
            return std::unexpected(stmt.error());

        if (!(match(NLINE) || match(SEMIC) || match(END)))
            return std::unexpected(Error::create(ErrorKind::ParseError,
                                                 SourceSpan(peek().span.position, 1),
                                                 "expected end of statement"));

        if (match(END))
        {
            stmts.push_back(std::move(stmt.value()));
            break;
        }

        auto _terminator = advance();

        // get rid of any trailing newlines
        while (match(NLINE))
        {
            auto _trail = advance();
        }

        stmts.push_back(std::move(stmt.value()));

        if (match(END))
            break;
    }

    return Expr(SequenceExpr(std::move(stmts)));
}

} // namespace tachyon::parser
