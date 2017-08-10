//
// Created by kevin on 04/08/16.
//

#include <iostream>
#include <vector>
#include <fstream>
#include <array>
#include <chrono>
#include "../csp/csp.h"

using namespace std;
using namespace std::chrono;
using namespace csp;

void prefix(unsigned long long value, chan_in<unsigned long long> in, chan_out<unsigned long long> out) noexcept
{
    out(value);
    while (true)
        out(in());
}

void delta(chan_in<unsigned long long> in, vector<chan_out<unsigned long long>> out) noexcept
{
    while (true)
    {
        auto value = in();
        par_for(out.begin(), out.end(), [&](chan_out<unsigned long long> chan){chan(value);});
    }
}

void successor(chan_in<unsigned long long> in, chan_out<unsigned long long> out) noexcept
{
    while (true)
    {
        auto val = in();
        out(++val);
    }
}

void consumer(chan_in<unsigned long long> in) noexcept
{
    unsigned long long x = 0;
    size_t loops = 100000;
    cout << "warming up..." << endl;
    for (size_t i = 0; i < loops; ++i)
        x = in();
    cout << "Last received: " << x << endl;

    array<unsigned long long, 1000> results;
    for (size_t count = 0; count < 1000; ++count)
    {
        auto start = system_clock::now();
        for (size_t i = 0; i < loops; ++i)
            x = in();
        auto end = system_clock::now();
        auto total = duration_cast<nanoseconds>(end - start).count();
        results[count] = total;
        cout << "." << endl;
    }

    ofstream result_file("commstime3.csv");
    for (size_t count = 0; count < 1000; ++count)
        result_file << results[count] << ",";
    result_file << endl;
}

int main(int argc, char **argv)
{
    one2one_chan<unsigned long long> a;
    one2one_chan<unsigned long long> b;
    one2one_chan<unsigned long long> c;
    one2one_chan<unsigned long long> d;

    par
    {
        make_proc(prefix, 0, c, a),
        make_proc(delta, a, vector<chan_out<unsigned long long>>{b, d}),
        make_proc(successor, b, c),
        make_proc(consumer, d)
    }();
}