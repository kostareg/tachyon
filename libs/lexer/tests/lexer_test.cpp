#include "tachyon/lexer/lexer.h"

#include <gtest/gtest.h>

using namespace tachyon::lexer;

TEST(LexerTest, RecognizesAllTokens) {
    Lexer lexed = lex("\n = + - -> * / ^ == != < > <= >= ! && || () ( ) { } [ ] . : ; , "
                      "\"hello, world!\" True False import "
                      "fn return someidentifier 12345");
    Tokens rhs_ts = lexed.tokens;
    std::vector<Value> rhs_cs = lexed.constants;

    // 1. assert constants
    std::vector<Value> lhs2 = {Value("hello, world!"), Value("someidentifier"), Value(12345.)};

    //   assert_eq size
    ASSERT_EQ(lhs2.size(), rhs_cs.size());

    //   assert_eq each value:
    for (size_t i = 0; i < rhs_cs.size(); ++i) {
        ASSERT_EQ(lhs2[i], rhs_cs[i]);
    }

    // 2. assert all token types.
    std::vector<TokenType> lhs_tts = {NLINE, EQ,     PLUS,   MINUS,  RARROW, STAR,   FSLASH, CARET,
                                      ECOMP, NECOMP, LCOMP,  GCOMP,  LECOMP, GECOMP, NOT,    BAND,
                                      BOR,   UNIT,   LPAREN, RPAREN, LBRACE, RBRACE, LBRACK, RBRACK,
                                      DOT,   COLON,  SEMIC,  COMMA,  STRING, TRUE,   FALSE,  IMPORT,
                                      FN,    RETURN, IDENT,  NUMBER, END};

    //   map vector<Token> to vector<TokenType>
    std::vector<TokenType> rhs_tts = {};
    std::ranges::transform(rhs_ts, std::back_inserter(rhs_tts),
                           [](const Token &t) -> TokenType { return t.type; });

    //   assert_eq whole vector
    ASSERT_EQ(lhs_tts, rhs_tts);
}

TEST(LexerTest, IgnoresComments) {
    // these lexed values should only include END.
    Tokens rhs_1 = lex("// comment with newline\n").tokens;
    Tokens rhs_2 = lex("// comment with EOF").tokens;
    Tokens rhs_3 = lex("/* multiline comment */").tokens;

    ASSERT_EQ(END, rhs_1.at(0).type);
    ASSERT_EQ(1, rhs_1.size());
    ASSERT_EQ(END, rhs_2.at(0).type);
    ASSERT_EQ(1, rhs_2.size());
    ASSERT_EQ(END, rhs_3.at(0).type);
    ASSERT_EQ(1, rhs_3.size());
}

// TODO: test for token positions