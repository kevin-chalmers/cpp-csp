#include <vector>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

using namespace testing;

TEST(SampleTest, Equal)
{
    EXPECT_EQ(42, 42);
}

int main(int argc, char **argv)
{
    InitGoogleTest(&argc, argv);
    int ret = RUN_ALL_TESTS();
    return ret;
}