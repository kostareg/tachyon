#pragma once

#include "ast/ast.hpp"
#include "lexer/lexer.hpp"

using namespace ast;
using namespace lexer;

namespace parser {
std::expected<ast::ExprRefs, Error> parse(Tokens);
} // namespace parser
