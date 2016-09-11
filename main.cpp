#include <iostream>
#include "csp/csp.h"
#include "csp/plugnplay/plugnplay.h"

using namespace std;
using namespace csp;
using namespace csp::plugnplay;

using proc = function<void()>;

int main(int argc, char**)
{
    one2one_chan<unsigned long long> a;

    par
    {
        [=](){a(1); *this();},
        proc B = [=](){auto x = a(); cout << x << endl; B();}
    }();

    return 0;
}