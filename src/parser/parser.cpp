#include "parser/parser.hpp"
#include "op.hpp"

namespace parser {

using enum TokenType;

std::unique_ptr<ASTNode> Parser::parse() {
    std::vector<std::unique_ptr<ASTNode>> statements;
    while (peek().type != END) {
        statements.push_back(statement());
    }
    return std::make_unique<SequenceNode>(std::move(statements));
};

std::unique_ptr<ASTNode> Parser::statement() {
    auto ident = matchRead(IDENT);
    if (ident && match(EQ)) {
        // x = <expr>;
        auto e = expr();
        expect(SEMIC);
        return std::make_unique<VariableDeclNode>(ident->ident, std::move(e));
    } else if (ident && match(LPAREN)) {
        // myfn(1, 2 * 5, x);
        std::vector<std::unique_ptr<ASTNode>> args;
        while (!match(RPAREN)) {
            // ... handle exprs
            args.push_back(expr());
            expect(SEMIC); // TODO: COMMA
        }

        expect(SEMIC);
        return std::make_unique<FunctionCallNode>(ident->ident, std::move(args));
    }

    throw std::runtime_error("ice: you shouldnt get here");
    return nullptr;
}

std::unique_ptr<ASTNode> Parser::expr() {
    // if we are the lhs of an operator
    // TODO: order of operations
    if (isoperator(peek(1).type)) {
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
        return std::make_unique<BinaryOperatorNode>(::Op::Mul, std::move(lhs), std::move(rhs));
    };

    auto n = matchRead(NUMBER);
    if (n) {
        return std::make_unique<NumberNode>(*n->value);
    }

    auto v = matchRead(IDENT); // could also be fn
    if (v)
        return std::make_unique<VariableRefNode>(v->ident);

    throw std::runtime_error("failed to parse expr");
}

std::unique_ptr<ASTNode> Parser::term() { return nullptr; }

std::unique_ptr<ASTNode> Parser::factor() { return nullptr; }
} // namespace parser
