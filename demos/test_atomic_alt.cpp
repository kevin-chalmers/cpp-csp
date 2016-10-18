//
// Created by kevin on 18/10/16.
//

#include <iostream>
#include "../csp/csp.h"

using namespace std;
using namespace csp;

void writer(chan_out<int> out)
{
    for (int i = 0; i < 100000; ++i)
        out(i);
}

void reader(alting_chan_in<int> in1, alting_chan_in<int> in2)
{
    alt a{in1, in2};
    for (int i = 0; i < 200000; ++i)
    {
        auto idx = a();
        cout << idx << " --> " << (idx == 0 ? in1() : in2()) << endl;
    }
}

int main(int argc, char **argv)
{
    busy_one2one_chan<int> a;
    busy_one2one_chan<int> b;

    par
    {
        make_proc(writer, a),
        make_proc(writer, b),
        make_proc(reader, a, b)
    }();
    return 0;
}