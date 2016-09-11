#include <iostream>
#include <vector>
#include <chrono>
#include "../csp/csp.h"
#include "../csp/plugnplay/plugnplay.h"

using namespace std;
using namespace std::chrono;
using namespace csp;
using namespace csp::plugnplay;

constexpr int N = 6;

using proc = function<void()>;

int main(int argc, char **argv)
{
    any2one_chan<string> report;

    auto FORK = [=](alting_chan_in<int> left, alting_chan_in<int> right)
    {
        while (true)
        {
            choice
            {
                {left, [=](){ left(); left(); } },
                {right, [=](){ right(); right(); } }
            }();
        }
    };

    auto PHIL = [=](int i, chan_out<int> left, chan_out<int> right, chan_out<int> down, chan_out<int> up)
    {
        timer t;
        while (true)
        {
            report(to_string(i) + " thinking");
            t(seconds(i));
            report(to_string(i) + " hungary");
            down(i);
            report(to_string(i) + " sitting");
            par_write({left, right}, {i, i});
            report(to_string(i) + " eating");
            t(seconds(i));
            report(to_string(i) + " leaving");
            par_write({left, right}, {i, i});
            up(i);
        }
    };

    auto SECURITY = [=](alting_chan_in<int> down, alting_chan_in<int> up)
    {
        int sitting = 0;
        while (true)
        {
            choice
            {
                {down, sitting < N - 1, [&](){ down(); ++sitting; }},
                {up, true, [&](){ up(); --sitting; }}
            }();
        }
    };

    one2one_chan<int> left[N], right[N];
    any2one_chan<int> down, up;
    vector<proc> fork(N);
    for (int i = 0; i < N; ++i)
        fork[i] = make_proc(FORK, left[i], right[(i +1)%N]);
    vector<proc> phil(N);
    for (int i = 0; i < N; ++i)
        phil[i] = make_proc(PHIL, i, left[i], right[i], down, up);
    par
    {
        par(phil),
        par(fork),
        make_proc(SECURITY, down, up),
        printer<string>(report, "", "")
    }();

    return 0;
}