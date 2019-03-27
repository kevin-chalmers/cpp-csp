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

TEST(Channel, Get_End)
{
    one2one_chan<int> a;
    one2any_chan<int> b;
    any2one_chan<int> c;
    any2any_chan<int> d;
    auto a_in = a.in();
    auto a_out = a.out();
    auto b_in = b.in();
    auto b_out = b.out();
    auto c_in = c.in();
    auto c_out = c.out();
    auto d_in = d.in();
    auto d_out = d.out();
}

TEST(Channel, Get_End_Type)
{
    one2one_chan<int> a;
    one2any_chan<int> b;
    any2one_chan<int> c;
    any2any_chan<int> d;
    auto a_in = a.in();
    auto a_out = a.out();
    auto b_in = b.in();
    auto b_out = b.out();
    auto c_in = c.in();
    auto c_out = c.out();
    auto d_in = d.in();
    auto d_out = d.out();
    ASSERT_EQ(typeid(a_in), typeid(guarded_chan_in<int>));
    ASSERT_EQ(typeid(a_out), typeid(chan_out<int>));
    ASSERT_EQ(typeid(b_in), typeid(shared_chan_in<int>));
    ASSERT_EQ(typeid(b_out), typeid(chan_out<int>));
    ASSERT_EQ(typeid(c_in), typeid(guarded_chan_in<int>));
    ASSERT_EQ(typeid(c_out), typeid(shared_chan_out<int>));
    ASSERT_EQ(typeid(d_in), typeid(shared_chan_in<int>));
    ASSERT_EQ(typeid(d_out), typeid(shared_chan_out<int>));
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