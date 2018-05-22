//
// Created by kevin on 22/05/18.
//

#include <iostream>
#include <fstream>
#include <chrono>
#include "../csp/csp.h"
#include "../csp/plugnplay/plugnplay.h"

using namespace std;
using namespace std::chrono;
using namespace csp;
using namespace csp::plugnplay;

class consumer : public process
{
private:

    chan_in<unsigned long long> _in;

public:
    consumer(chan_in<unsigned long long> in) noexcept
            : _in(in)
    {
    }

    void run() noexcept override final
    {
        unsigned long long x = 0;
        size_t loops = 100000;
        cout << "warming up..." << endl;
        for (size_t i = 0; i < loops; ++i)
            x = _in();
        cout << "Last received: " << x << endl;

        array<unsigned long long, 1000> results;
        for (size_t count = 0; count < 1000; ++count)
        {
            auto start = system_clock::now();
            for (size_t i = 0; i < loops; ++i)
                x = _in();
            auto end = system_clock::now();
            auto total = duration_cast<nanoseconds>(end - start).count();
            results[count] = total;
            cout << count << endl;
        }

        ofstream result_file("fibercommstime1.csv");
        for (size_t count = 0; count < 1000; ++count)
            result_file << results[count] << ",";
        result_file << endl;
    }
};

int main(int argc, char **argv)
{
    fiber_one2one_chan<unsigned long long> a;
    fiber_one2one_chan<unsigned long long> b;
    fiber_one2one_chan<unsigned long long> c;
    fiber_one2one_chan<unsigned long long> d;

    fiber_par
    {
        prefix<unsigned long long>(0, c, a),
        delta<unsigned long long, true>(a, {b, d}),
        successor<unsigned long long>(b, c),
        consumer(d)
    }();

    return 0;
}