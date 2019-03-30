// Copyright 2019 Kevin Chalmers
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <csp/csp.h>

using csp::One2OneChan;
using csp::One2AnyChan;
using csp::Any2OneChan;
using csp::Any2AnyChan;
using csp::GuardedChanIn;
using csp::SharedChanIn;
using csp::ChanOut;
using csp::SharedChanOut;

/*!
 \brief Tests that channels can be created with no exceptions.
 */
TEST(Channel, Create) {
    One2OneChan<int> a;
    One2AnyChan<int> b;
    Any2OneChan<int> c;
    Any2AnyChan<int> d;
}

/*!
 \brief Tests that channel ends can be retrieved with no exceptions.
 */
TEST(Channel, Get_End) {
    One2OneChan<int> a;
    One2AnyChan<int> b;
    Any2OneChan<int> c;
    Any2AnyChan<int> d;
    auto a_in = a.in();
    auto a_out = a.out();
    auto b_in = b.in();
    auto b_out = b.out();
    auto c_in = c.in();
    auto c_out = c.out();
    auto d_in = d.in();
    auto d_out = d.out();
}

/*!
 \brief Testss that the returned input and output ends are the correct type.
 */
TEST(Channel, Get_End_Type) {
    One2OneChan<int> a;
    One2AnyChan<int> b;
    Any2OneChan<int> c;
    Any2AnyChan<int> d;
    auto a_in = a.in();
    auto a_out = a.out();
    auto b_in = b.in();
    auto b_out = b.out();
    auto c_in = c.in();
    auto c_out = c.out();
    auto d_in = d.in();
    auto d_out = d.out();
    ASSERT_EQ(typeid(a_in), typeid(GuardedChanIn<int>));
    ASSERT_EQ(typeid(a_out), typeid(ChanOut<int>));
    ASSERT_EQ(typeid(b_in), typeid(SharedChanIn<int>));
    ASSERT_EQ(typeid(b_out), typeid(ChanOut<int>));
    ASSERT_EQ(typeid(c_in), typeid(GuardedChanIn<int>));
    ASSERT_EQ(typeid(c_out), typeid(SharedChanOut<int>));
    ASSERT_EQ(typeid(d_in), typeid(SharedChanIn<int>));
    ASSERT_EQ(typeid(d_out), typeid(SharedChanOut<int>));
}
