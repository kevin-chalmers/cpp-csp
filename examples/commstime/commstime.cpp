#include <iostream>
#include <csp/csp.hpp>

using namespace std;
using namespace csp;

class prefix : public process
{
    int _prefix;
    chan_in<int> _in;
    chan_out<int> _out;

public:
    prefix(int prefix, chan_in<int> in, chan_out<int> out)
    : _prefix(prefix), _in(move(in)), _out(move(out))
    {
    }

    void run() noexcept final
    {
        _out(_prefix);
        while (true)
            _out(_in());
    }
};

class delta : public process
{
    chan_in<int> _in;
    chan_out<int> _out1;
    chan_out<int> _out2;

public:
    delta(chan_in<int> in, chan_out<int> out1, chan_out<int> out2)
    : _in(move(in)), _out1(move(out1)), _out2(move(out2))
    {
    }

    void run() noexcept final
    {
        while (true)
        {
            auto n = _in();
            _out1(n);
            _out2(n);
        }
    }
};

class succ : public process
{
    chan_in<int> _in;
    chan_out<int> _out;

public:
    succ(chan_in<int> in, chan_out<int> out)
    : _in(move(in)), _out(move(out))
    {
    }

    void run() noexcept final
    {
        while (true)
        {
            auto n = _in();
            _out(n + 1);
        }
    }
};

class printer : public process
{
    chan_in<int> _in;

public:
    printer(chan_in<int> in)
    : _in(in)
    {
    }

    void run() noexcept final
    {
        while (true)
            cout << _in() << endl;
    }
};

int main(int argc, char **argv) noexcept
{
    // Select concurrency model to use
    using model = thread_model;
    // Create communication network and run
    auto a = model::make_one2one<int>();
    auto b = model::make_one2one<int>();
    auto c = model::make_one2one<int>();
    auto d = model::make_one2one<int>();
    parallel<model>
    {
        make_proc<prefix>(0, c, a),
        make_proc<delta>(a, b, d),
        make_proc<succ>(b, c),
        make_proc<printer>(d)
    }();
    return 0;
}