#pragma once

#include "ast/ast.hpp"
#include "lexer/lexer.hpp"

using namespace ast;
using namespace lexer;

namespace parser {
std::expected<ast::Exprs, Error> parse(Tokens);
} // namespace parser
