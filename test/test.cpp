#include <vector>
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <csp/csp.hpp>

using namespace testing;
using namespace csp;

TEST(Channel, Create)
{
    one2one_chan<int> a;
    one2any_chan<int> b;
    any2one_chan<int> c;
    any2any_chan<int> d;
}

TEST(Parallel, Create)
{
    parallel p;
}

int main(int argc, char **argv)
{
    InitGoogleTest(&argc, argv);
    int ret = RUN_ALL_TESTS();
    return ret;
}