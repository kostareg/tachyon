#pragma once

#include "tachyon/common/error.h"
#include "tachyon/lexer/token.h"

#include <expected>

namespace tachyon::lexer
{
// TODO: take string_view here?
/**
 * @brief lex string into list of tokens
 * @return tokens or error
 */
std::expected<Tokens, Error> lex(const std::string &);
} // namespace tachyon::lexer