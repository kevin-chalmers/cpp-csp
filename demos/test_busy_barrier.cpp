//
// Created by kevin on 22/10/16.
//

#include <iostream>
#include <functional>
#include "../csp/csp.h"

using namespace std;
using namespace csp;

constexpr int NUM_PROCS = 16;

void syncer(int idx, barrier b)
{
    while (true)
    {
        cout << idx << " sycning" << endl;
        b();
        cout << idx << " synced" << endl;
    }
}

int main(int argc, char **argv)
{
    busy_barrier b(NUM_PROCS);

    vector<function<void()>> procs;
    for (int i = 0; i < NUM_PROCS; ++i)
        procs.push_back(make_proc(syncer, i, b));

    par p(procs);
    p();

    return 0;
}