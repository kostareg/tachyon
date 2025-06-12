#include "tachyon/lexer/lexer.h"

#include "gtest/gtest.h"

using namespace tachyon::lexer;

TEST(LexerTest, RecognizesAllTokens)
{
    auto rhs_ts = lex("\n = + - -> * / > ^ () ( ) { } . : ; , "
                      "\"hello, world!\" True False import "
                      "fn return someidentifier 12345")
                      .value();

    // 1. assert all values. default to std::monostate{}:
    std::vector<std::variant<std::monostate, double, bool, std::string>> lhs2(rhs_ts.size(),
                                                                              std::monostate{});

    //   with some exceptional variants:
    lhs2[18] = "hello, world!";
    lhs2[19] = true;
    lhs2[20] = false;
    lhs2[24] = "someidentifier";
    lhs2[25] = 12345.;

    //   assert_eq each value:
    for (size_t i = 0; i < lhs2.size(); ++i)
    {
        ASSERT_EQ(lhs2[i], rhs_ts[i].value);
    }

    // 2. assert all token types.
    std::vector<TokenType> lhs_tts = {NLINE,  EQ,    PLUS,   MINUS,  RARROW, STAR,   FSLASH,
                                      RCHEV,  CARET, UNIT,   LPAREN, RPAREN, LBRACE, RBRACE,
                                      DOT,    COLON, SEMIC,  COMMA,  STRING, BOOL,   BOOL,
                                      IMPORT, FN,    RETURN, IDENT,  NUMBER, END};

    //   map vector<Token> to vector<TokenType>
    std::vector<TokenType> rhs_tts = {};
    std::ranges::transform(rhs_ts, std::back_inserter(rhs_tts),
                           [](const Token &t) -> TokenType { return t.type; });

    //   assert_eq whole vector
    ASSERT_EQ(lhs_tts, rhs_tts);
}

TEST(LexerTest, IgnoresComments)
{
    // these lexed values should only include END.
    auto rhs_1 = lex("// hello, world!\n").value();
    auto rhs_2 = lex("/* this is a comment */").value();

    ASSERT_EQ(END, rhs_1.at(0).type);
    ASSERT_EQ(1, rhs_1.size());
    ASSERT_EQ(END, rhs_2.at(0).type);
    ASSERT_EQ(1, rhs_2.size());
}

// TODO: test for token positions