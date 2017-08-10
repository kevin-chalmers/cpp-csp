//
// Created by kevin on 18/10/16.
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

using proc = function<void()>;

class consumer : public process
{
private:
    vector<chan_in<unsigned long long>> _in;

public:
    consumer(vector<chan_in<unsigned long long>> in) noexcept
            : _in(in)
    {
    }

    void run() noexcept override final
    {
        unsigned long long x = 0;
        size_t loops = 100;
        cout << "warming up..." << endl;
        for (size_t i = 0; i < loops; ++i)
            for (size_t c = 0; c < _in.size(); ++c)
                x = _in[c]();
        cout << "Last received: " << x << endl;

        array<unsigned long long, 100> results;
        for (size_t count = 0; count < 100; ++count)
        {
            auto start = system_clock::now();
            for (size_t i = 0; i < loops; ++i)
                for (size_t c = 0; c < _in.size(); ++c)
                    x = _in[c]();
            auto end = system_clock::now();
            auto total = duration_cast<nanoseconds>(end - start).count();
            results[count] = total;
            cout << count << endl;
        }
        ofstream result_file(string("busycommstime" + to_string(_in.size()) + string(".csv")));
        for (size_t count = 0; count < 100; ++count)
            result_file << results[count] << ",";
        result_file << endl;
    }
};

int main(int argc, char **argv)
{
    int count = 2;
    if (argc >= 2)
        count = atoi(argv[1]);

    vector<busy_one2one_chan<unsigned long long>> a(count);
    vector<busy_one2one_chan<unsigned long long>> b(count);
    vector<busy_one2one_chan<unsigned long long>> c(count);
    vector<busy_one2one_chan<unsigned long long>> d(count);

    for (size_t i = 0; i < count; ++i)
    {
        a[i] = busy_one2one_chan<unsigned long long>();
        b[i] = busy_one2one_chan<unsigned long long>();
        c[i] = busy_one2one_chan<unsigned long long>();
        d[i] = busy_one2one_chan<unsigned long long>();
    }

    vector<proc> procs;
    for (size_t i = 0; i < count; ++i)
    {
        procs.push_back(prefix<unsigned long long>(0, c[i], a[i]));
        procs.push_back(delta<unsigned long long, true>(a[i], {b[i], d[i]}));
        procs.push_back(successor<unsigned long long>(b[i], c[i]));
    }
    procs.push_back(consumer(vector<chan_in<unsigned long long>>(d.begin(), d.end())));

    par p(procs);
    p();

    return 0;
}