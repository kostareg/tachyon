#include "parser/parser.hpp"
#include "op.hpp"

namespace parser {

using enum TokenType;

std::expected<std::unique_ptr<ASTNode>, Error> Parser::parse() {
    std::vector<std::unique_ptr<ASTNode>> statements;
    while (peek().type != END) {
        auto s = statement();
        if (!s.has_value())
            return s;
        statements.push_back(std::move(s.value()));
    }
    return std::make_unique<SequenceNode>(std::move(statements));
};

std::expected<std::unique_ptr<ASTNode>, Error> Parser::statement() {
    auto ident = matchRead(IDENT);
    if (ident && match(EQ)) {
        // x = <expr>;
        auto e = expr();
        if (!e.has_value())
            return e;
        auto s = expect(SEMIC);
        if (!s.has_value())
            return std::unexpected(s.error());
        return std::make_unique<VariableDeclNode>(ident->ident,
                                                  std::move(e.value()));
    } else if (ident && match(LPAREN)) {
        // myfn(1, 2 * 5, x);
        std::vector<std::unique_ptr<ASTNode>> args;
        bool needsDelim = false;
        while (!match(RPAREN)) {
            if (needsDelim) {
                auto c = expect(COMMA);
                if (!c.has_value())
                    return std::unexpected(c.error());
            } else {
                needsDelim = true;
            }
            // ... handle exprs
            auto e = expr();
            if (!e.has_value())
                return e;
            args.push_back(std::move(e.value()));
        }

        auto s = expect(SEMIC);
        if (!s.has_value())
            return std::unexpected(s.error());
        return std::make_unique<FunctionCallNode>(ident->ident,
                                                  std::move(args));
    }

    auto next = peek();
    return std::unexpected(
        Error(ErrorKind::ParseError, "malformed statement", next.m.pos,
              next.m.line, next.m.col, next.m.len)
            .with_code("E0004")
            .with_hint("this is a syntax error.")
            .with_hint("there is no way for me to handle this statement."));
}

std::expected<std::unique_ptr<ASTNode>, Error> Parser::expr() {
    // if we are the lhs of an operator
    // TODO: order of operations
    if (isoperator(peek(1).type)) {
        ::Op op = tok_to_op(peek(1).type);
        auto l = advance();
        std::unique_ptr<ASTNode> lhs;
        if (l.type == NUMBER) {
            lhs = std::make_unique<NumberNode>(*l.value);
        } else if (l.type == IDENT) {
            lhs = std::make_unique<VariableRefNode>(l.ident);
        } else {
            throw std::runtime_error("ice: still working around this");
        }

        advance(); // operator
        auto rhs = expr();
        if (!rhs.has_value())
            return std::unexpected(rhs.error());
        return std::make_unique<BinaryOperatorNode>(op, std::move(lhs),
                                                    std::move(rhs.value()));
    };

    auto n = matchRead(NUMBER);
    if (n) {
        return std::make_unique<NumberNode>(*n->value);
    }

    auto v = matchRead(IDENT); // could also be fn
    if (v)
        return std::make_unique<VariableRefNode>(v->ident);

    auto next = peek();
    return std::unexpected(
        Error(ErrorKind::ParseError, "could not parse expression", next.m.pos,
              next.m.line, next.m.col, next.m.len)
            .with_code("E0005")
            .with_hint("this is a syntax error.")
            .with_hint("there is no way for me to handle this expression."));
}
} // namespace parser
