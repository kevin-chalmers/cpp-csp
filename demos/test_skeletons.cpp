//
// Created by kevin on 15/11/16.
//

#include <iostream>
#include "../csp/skeletons/skeletons.h"

using namespace std;
using namespace csp;
using namespace csp::skeletons;

class my_source : public source<int>
{
public:
    my_source() noexcept
    : source<int>()
    {
    }

    void run() noexcept override final
    {
        for (int i = 0; i < 10; ++i)
            send(i);
    }
};

class my_sink : public sink<int>
{
public:
    my_sink() noexcept
    : sink<int>()
    {
    }

    void run() noexcept override final
    {
        for (int i = 0; i < 10; ++i)
            cout << recv() << endl;
    }
};

int sqr(int n) noexcept { return n * n; }

int main(int argc, char **argv)
{
    one2one_chan<int> c[2];
    my_source source;
    my_sink sink;
    wrapper<int, int> wrap(sqr);
    source.set_send(c[0]);
    wrap.set_recv(c[0]);
    wrap.set_send(c[1]);
    sink.set_recv(c[1]);
    par
    {
        make_proc(source),
        make_proc(wrap),
        make_proc(sink)
    }();

    return 0;
}