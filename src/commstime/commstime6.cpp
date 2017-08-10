//
// Created by kevin on 04/08/16.
//

#include <iostream>
#include <vector>
#include <array>
#include <fstream>
#include <chrono>
#include <csp/csp.h>

using namespace std;
using namespace std::chrono;
using namespace csp;

int main(int argc, char **argv)
{
    one2one_chan<unsigned long long> a;
    one2one_chan<unsigned long long> b;
    one2one_chan<unsigned long long> c;
    one2one_chan<unsigned long long> d;

    par
    {
        [=]()
        {
            a(0);
            while (true)
                a(c());
        },
        [=]()
        {
            while (true)
            {
                auto value = a();
                b(value);
                d(value);
            }
        },
        [=]()
        {
            while (true)
            {
                auto value = b();
                c(++value);
            }
        },
        [=]()
        {
            unsigned long long x = 0;
            size_t loops = 100000;
            cout << "warming up..." << endl;
            for (size_t i = 0; i < loops; ++i)
                x = d();
            cout << "Last received: " << x << endl;

            array<unsigned long long, 1000> results;
            for (size_t count = 0; count < 1000; ++count)
            {
                auto start = system_clock::now();
                for (size_t i = 0; i < loops; ++i)
                    x = d();
                auto end = system_clock::now();
                auto total = duration_cast<nanoseconds>(end - start).count();
                results[count] = total;
                cout << "." << endl;
            }

            ofstream result_file("commstime6.csv");
            for (size_t count = 0; count < 1000; ++count)
                result_file << results[count] << ",";
            result_file << endl;
        }
    }();
}