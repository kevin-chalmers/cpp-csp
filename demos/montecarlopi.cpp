//
// Created by kevin on 04/08/16.
//

#include <iostream>
#include <string>
#include <array>
#include <fstream>
#include <chrono>
#include <random>
#include "../csp/csp.h"

using namespace std;
using namespace std::chrono;
using namespace csp;

unsigned int NUM_WORKERS = 1;
constexpr unsigned int ITERATIONS = 1 << 30;

void monte_carlo_pi(chan_out<double> out, unsigned int iterations) noexcept
{
    auto millis = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
    default_random_engine e(millis.count());
    uniform_real_distribution<double> distribution(0.0, 1.0);

    unsigned int in_circle = 0;
    for (unsigned int i = 0; i < iterations; ++i)
    {
        auto x = distribution(e);
        auto y = distribution(e);
        auto length = pow(x, 2.0) + pow(y, 2.0);
        if (length <= 1.0)
            ++in_circle;
    }
    auto pi = (4.0 * in_circle) / static_cast<double>(iterations);
    out(pi);
}

void calculate(chan_in<double> in, unsigned int num_workers) noexcept
{
    double sum = 0;
    for (unsigned int i = 0; i < num_workers; ++i)
    {
        sum += in();
    }
    sum /= static_cast<double>(num_workers);
    cout << "pi: " << sum << endl;
}

int main(int argc, char **argv)
{
    if (argc == 2)
        NUM_WORKERS = stoi(argv[1]);
    auto iter_worker = ITERATIONS / NUM_WORKERS;
    // Warming up
    any2one_chan<double> chan;
    vector<function<void()>> workers;
    for (int count = 0; count < NUM_WORKERS; ++count)
        workers.push_back(make_proc(monte_carlo_pi, chan, iter_worker));
    cout << "Warming up..." << endl;
    for (int i = 0; i < 10; ++i)
    {
        par
        {
            par(workers),
            make_proc(calculate, chan, NUM_WORKERS)
        }();
    }

    array<unsigned long long, 100> res;
    for (unsigned int i = 0; i < 100; ++i)
    {
        auto start = system_clock::now();
        par
        {
            par(workers),
            make_proc(calculate, chan, NUM_WORKERS)
        }();
        auto stop = system_clock::now();
        res[i] = duration_cast<nanoseconds>(stop - start).count();
        cout << i << ": " << res[i] << endl;
    }
    ofstream results("montecarlopi_" + to_string(NUM_WORKERS) + ".csv");
    for (unsigned int i = 0; i < 100; ++i)
        results << res[i] << ",";
    results.close();
    return 0;
}