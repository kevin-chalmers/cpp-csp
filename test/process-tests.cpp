//
// Created by kevin on 30/03/19.
//

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <csp/csp.hpp>

using namespace testing;
using namespace csp;

/*!
 \brief Tests that a parallel can be created with no exceptions.
 */
TEST(Parallel, Create)
{
    parallel p;
}