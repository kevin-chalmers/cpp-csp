#include <iostream>
#include <vector>
#include <chrono>
#include <csp/csp.hpp>

using namespace std;
using namespace std::chrono;
using namespace csp;

class fairplex : public process
{
    vector<guarded_chan_in<int>> _in;
    chan_out<int> _out;
public:
    fairplex(vector<guarded_chan_in<int>> &in, chan_out<int> out)
    : _in(move(in)), _out(move(out))
    {
    }

    void run() noexcept final
    {
        auto alt = make_alt(_in);
        while (true)
        {
            // Can be done in one line - but its ugly!
            // _out(_in[alt()]());
            auto idx = alt();
            auto c = _in[idx];
            auto v = c();
            _out(v);
        }
    }
};

class regular : public process
{
    chan_out<int> _out;
    const int _N;
    const system_clock::duration _interval;
public:
    regular(chan_out<int> out, int N, system_clock::duration interval)
    : _out(move(out)), _N(N), _interval(interval)
    {
    }

    void run() noexcept final
    {
        timer<system_clock> tim;
        auto timeout = tim();
        while (true)
        {
            _out(_N);
            timeout += _interval;
            sleep(timeout);
        }
    }
};

int main(int argc, char **argv)
{
    using model = thread_model;
    // TODO: Add channel list creation methods.
}