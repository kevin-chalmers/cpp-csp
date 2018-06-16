#include <iostream>
#include <vector>
#include <chrono>
#include <fstream>
#include <csp/csp.hpp>

using namespace std;
using namespace std::chrono;
using namespace csp;

class writer final : public process
{
private:
    chan_out<int> out;
public:
    explicit writer(chan_out<int> out)
    : out(move(out))
    {
    }

    void run() noexcept
    {
        for (int i = 1; i <= (1 << 16); ++i)
            out(i);
    }
};

class reader final : public process
{
private:
    vector<guarded_chan_in<int>> in;
    int power;
public:
    explicit reader(vector<guarded_chan_in<int>> in, int power)
    : in(move(in)), power(power)
    {
    }

    void run() noexcept
    {
        vector<guard> g(in.begin(), in.end());
        auto alt = make_alt(g);
        ofstream res(string("stressed.") + to_string(in.size()) + string(".csv"));
        for (size_t i = 0; i < ((1 << 16) * in.size()); i += (1 << (10 + power)))
        {
            auto start = system_clock::now();
            for (size_t j = 0; j < (1 << (10 + power)); ++j)
            {
                auto idx = alt();
                auto v = in[idx]();
            }
            auto total = (system_clock::now() - start).count();
            cout << in.size() << " " << ((1 << 16) * in.size()) - i << endl;
            res << static_cast<double>(total) / (1 << (10 + power)) << ",";
        }
    }
};

int main(int argc, char **argv) noexcept
{
    boost::fibers::use_scheduling_algorithm<boost::fibers::algo::round_robin>();
    using model = fiber_model;
    for (size_t w = 1; w <= 24; ++w)
    {
        vector<proc_t> procs;
        vector<guarded_chan_in<int>> chans;
        for (size_t i = 0; i < (1 << w); ++i)
        {
            auto c = primitive_builder::make_one2one<int>(model::model_type);
            procs.push_back(make_proc<writer>(c));
            chans.push_back(c);
        }
        procs.push_back(make_proc<reader>(move(chans), w));
        parallel<model>(procs).run();
    }
    return 0;
}