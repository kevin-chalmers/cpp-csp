//
// Created by kevin on 15/11/16.
//

#include <iostream>
#include "../csp/skeletons/skeletons.h"

using namespace std;
using namespace csp;
using namespace csp::skeletons;

void sourcer(chan_out<int> send) noexcept
{
    for (int i = 0; i < 10; ++i)
        send(i);
}

void sinker(chan_in<int> recv) noexcept
{
    for (int i = 0; i < 10; ++i)
        cout << recv() << endl;
}

int sqr(int n) noexcept { return n * n; }

int main(int argc, char **argv)
{
    source<int> source(sourcer);
    sink<int> sink(sinker);
    auto block = source + sink;
    block();

    return 0;
}