#include <iostream>
#include <array>
#include <csp/csp.hpp>

using namespace std;
using namespace csp;

class polling : public process
{
    array<guarded_chan_in<int>, 5> _in;
    chan_out<int> _out;

public:
    polling(array<guarded_chan_in<int>, 5> in, chan_out<int> _out)
    : _in(move(in)), _out(move(out))
    {
    }

    void run() final
    {

    }
};