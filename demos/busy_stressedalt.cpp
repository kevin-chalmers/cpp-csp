//
// Created by kevin on 01/11/16.
//

#include <iostream>
#include <fstream>
#include <chrono>
#include <array>
#include "../csp/csp.h"

using namespace std;
using namespace std::chrono;
using namespace csp;

unsigned int CHANNELS = 1;
unsigned int WRITERS_PER_CHANNEL = 2;

struct stressed_packet
{
    unsigned int writer;
    unsigned int n;
};

void stressed_writer(chan_out<stressed_packet> out, unsigned int writer)
{
    unsigned int n = 0;
    while (true)
    {
        out({ writer, n });
        ++n;
    }
}

void stressed_reader(vector<alting_chan_in<stressed_packet>> c, unsigned int writers_per_channel)
{
    timer t;
    cout << "Wait 5 seconds for all writers to get going..." << endl;
    t(seconds(5));
    cout << "OK - that should be long enough..." << endl;
    // Vector to hold last read values
    vector<vector<unsigned int>> n;
    for (unsigned int i = 0; i < c.size(); ++i)
        n.push_back(vector<unsigned int>(writers_per_channel));

    for (unsigned int chan = 0; chan < c.size(); ++chan)
    {
        for (unsigned int i = 0; i < writers_per_channel; ++i)
        {
            auto pckt = c[chan]();
            n[chan][pckt.writer] = pckt.n;
            for (unsigned int x = 0; x < chan; ++x) cout << "  ";
            cout << "channel " << chan << " writer " << pckt.writer << " read " << pckt.n << endl;
        }
    }
    // Create alt.  Need to convert vector of alting channels
    alt a(vector<guard>(c.begin(), c.end()));
    unsigned int counter = 10000, tock = 0;
    auto start = system_clock::now();
    auto stop = system_clock::now();
    array<unsigned long long, 1000> results;
    ofstream res("stressedalt_" + to_string(CHANNELS) + "_" + to_string(WRITERS_PER_CHANNEL) + ".csv");
    while (tock < 1000)
    {
        if (counter == 0)
        {
            stop = system_clock::now();
            auto total = duration_cast<nanoseconds>(stop - start).count();
            results[tock] = total / 10000;
            cout << "tock " << tock << " : " << total << endl;
            ++tock;
            counter = 10000;
            start = system_clock::now();
        }
        --counter;
        auto idx = a();
        auto pckt = c[idx]();
        n[idx][pckt.writer] = pckt.n;
    }
    for (unsigned int i = 0; i < 1000; ++i)
        res << results[i] << ",";
    res.close();
}

int main(int argc, char** argv)
{
    if (argc == 3)
    {
        CHANNELS = stoi(argv[1]);
        WRITERS_PER_CHANNEL = stoi(argv[2]);
    }
    cout << CHANNELS << " : " << WRITERS_PER_CHANNEL << endl;

    vector<busy_any2one_chan<stressed_packet>> c(CHANNELS);
    vector<function<void()>> procs;
    for (unsigned int i = 0; i < CHANNELS; ++i)
        for (unsigned int j = 0; j < WRITERS_PER_CHANNEL; ++j)
            procs.push_back(make_proc(stressed_writer, c[i], j));

    par
    {
            par(procs),
            make_proc(stressed_reader, vector<alting_chan_in<stressed_packet>>(c.begin(), c.end()), WRITERS_PER_CHANNEL)
    }();
}