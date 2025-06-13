#include "tachyon/common/error.h"

#include <gtest/gtest.h>

using namespace tachyon;

TEST(ErrorTest, FormatsInternalErrorMesage)
{
    std::string result =
        "\x1B[31merror:\x1B[0m encountered internal error on L1C0: \n"
        "some error\n"
        "\n"
        "1 | hello, world!\n"
        "\n"
        "\x1B[31mTHIS IS AN INTERNAL ERROR AND IS A BUG IN TACHYON. PLEASE REPORT THIS AT "
        "www.github.com/kostareg/tachyon/issues.\x1B[0m\n";

    Error error = Error::create(ErrorKind::InternalError, SourceSpan(0, 0), "some error");
    error.source = "hello, world!";

    std::ostringstream buffer;
    buffer << error;

    ASSERT_EQ(result, buffer.str());
}

// TODO: more
// TODO: there was some kind of testing framework that made this easy? does it integrate with gtest?
//  research.