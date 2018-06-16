#include <iostream>
#include <fstream>
#include <chrono>
#include <csp/csp.hpp>

using namespace std;
using namespace std::chrono;
using namespace csp;

void syncer(barrier bar)
{
    bar();
    for (size_t i = 0; i < (1 << 16); ++i)
        bar();
}

void global_syncer(barrier global_bar, barrier local_bar)
{
    global_bar();
    local_bar();
    for (size_t i = 0; i < (1 << 16); ++i)
    {
        local_bar();
        global_bar();
    }
}

class controller final : public process
{
private:
    size_t num;
    barrier global_bar;
public:
    controller(size_t num, barrier global_bar)
    : num(num), global_bar(global_bar)
    {
    }

    void run() noexcept
    {
        auto local_bar = make_bar(num + 1);
        vector<proc_t> procs;
        for (size_t i = 0; i < num; ++i)
            procs.push_back(make_proc<process_function>(bind(syncer, local_bar)));
        procs.push_back(make_proc<process_function>(bind(global_syncer, global_bar, local_bar)));
        par(procs);
    }
};

class timer final : public process
{
private:
    barrier global_bar;
    size_t threads;
    size_t num;
public:
    timer(barrier global_bar, size_t threads, size_t num)
    : global_bar(global_bar)
    {
    }

    void run() noexcept
    {
        ofstream res(string("stressed.bar.") + to_string(threads) + string(".") + to_string(num) + string(".csv"));
        global_bar();
        for (size_t i = 0; i < (1 << 16); i += (1 << 10))
        {
            auto start = system_clock::now();
            for (size_t j = 0; j < (1 << 10); ++j)
            {
                global_bar();
            }
            auto total = (system_clock::now() - start).count();
            cout << i << endl;
            res << static_cast<double>(total) / (1 << 10) << ",";
        }
    }
};

int main(int argc, char **argv) noexcept
{
    using model = atomic_fiber_model;
    for (size_t i = 1; i <= 3; ++i)
    {
        auto global_bar = model::make_bar(i + 1);
        for (size_t j = 1; j <= (1 << 16); j <<= 1)
        {
            cout << i << " " << j << endl;
            vector<proc_t> procs;
            for (size_t count = 0; count < i; ++count)
            {
                procs.push_back(make_proc<controller>(j, global_bar));
            }
            procs.push_back(make_proc<timer>(global_bar, i, j));
            parallel<model>(procs).run();
        }
    }
}