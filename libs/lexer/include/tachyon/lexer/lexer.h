#pragma once

#include "tachyon/common/error.h"
#include "tachyon/lexer/token.h"

#include <expected>

namespace tachyon::lexer
{
std::expected<Tokens, Error> lex(const std::string &);
}