#pragma once

#include "tachyon/common/error.h"
#include "tachyon/lexer/token.h"

// TODO: fix this, some other name or merge with runtime::Value in common
using Value = std::variant<std::monostate, double, std::string, bool>;

namespace tachyon::lexer {
class Lexer {
  public:
    std::vector<Token> tokens;
    std::vector<Value> constants;
    std::vector<Error> errors;
    const char *current = nullptr;

    Lexer() {
        // since Tachyon cares much more about performance than memory size, let's reserve some
        // slightly above average sizes for tokens and constants.
        tokens.reserve(20000);
        constants.reserve(10000);
    }

    void lex(const std::string &);

  private:
    inline void consume_and_push(TokenType tt, size_t length = 1);
    inline void consume_and_push(TokenType tt, Value constant, size_t length = 1);
};

inline Lexer lex(const std::string &source_code) {
    Lexer lexer{};
    lexer.lex(source_code);
    return lexer;
}
} // namespace tachyon::lexer