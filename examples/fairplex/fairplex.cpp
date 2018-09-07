#include <iostream>
#include <vector>
#include <csp/csp.hpp>

using namespace std;
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

    void run() final
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
    const long _interval;
public:
    regular(chan_out<int> out, int N, long interval)
    : _out(move(out)), _N(N), _interval(interval)
    {
    }

    void run() final
    {

    }
};