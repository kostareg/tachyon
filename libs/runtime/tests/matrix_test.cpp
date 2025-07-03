#include "tachyon/runtime/matrix.h"

#include <gtest/gtest.h>

using namespace tachyon;

TEST(MatrixTest, RowInit)
{
    runtime::Matrix m({1, 2, 3, 4, 5, 6});

    ASSERT_EQ(m(1, 1), 1);
    ASSERT_EQ(m(1, 2), 2);
    ASSERT_EQ(m(1, 3), 3);
    ASSERT_EQ(m(1, 4), 4);
    ASSERT_EQ(m(1, 5), 5);
    ASSERT_EQ(m(1, 6), 6);
}

TEST(MatrixTest, MatrixInit)
{
    runtime::Matrix m({1, 2, 3, 4, 5, 6}, 3);

    ASSERT_EQ(m(1, 1), 1);
    ASSERT_EQ(m(1, 2), 2);
    ASSERT_EQ(m(1, 3), 3);
    ASSERT_EQ(m(2, 1), 4);
    ASSERT_EQ(m(2, 2), 5);
    ASSERT_EQ(m(2, 3), 6);
}

TEST(MatrixTest, Assignment)
{
    runtime::Matrix m({1, 2, 3, 4, 5, 6}, 2);
    m(1, 2) = 10;
    m(5) = 15;

    ASSERT_EQ(m(1, 1), 1);
    ASSERT_EQ(m(1, 2), 10);
    ASSERT_EQ(m(2, 1), 3);
    ASSERT_EQ(m(2, 2), 4);
    ASSERT_EQ(m(3, 1), 15);
    ASSERT_EQ(m(3, 2), 6);
}

#ifdef TY_DEBUG
TEST(MatrixTest, DeathUnsquareMatrix)
{
    EXPECT_DEATH(runtime::Matrix m({1, 2, 3, 4, 5, 6}, 4), "matrix is square");
}
#endif