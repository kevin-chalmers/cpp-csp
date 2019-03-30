#include <gtest/gtest.h>

using namespace testing;

int main(int argc, char **argv)
{
    InitGoogleTest(&argc, argv);
    int ret = RUN_ALL_TESTS();
    return ret;
}