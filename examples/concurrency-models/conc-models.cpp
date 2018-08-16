#include <iostream>
#include <vector>
#include <csp/csp.hpp>

using namespace std;
using namespace csp;

// Number of child and childs per child.
constexpr size_t PARENTS = 4;
constexpr size_t CHILDREN = 100;

// The bottom process
class child : public process
{
    barrier _bar;
public:
    child(barrier bar)
    : _bar(move(bar))
    {
    }

    void run() noexcept final
    {
        // Sync 1000 times on the bar
        for (auto j = 0; j < 1000; ++j)
            _bar();
    }
};

class parent : public process
{
    size_t _idx;
    barrier _main_bar;
public:
    parent(size_t idx, barrier main_bar)
    : _idx(idx), _main_bar(main_bar)
    {
    }

    void run() noexcept final
    {
        // Create a barrier
        auto bar = make_bar(CHILDREN);
        // Create child processes
        vector<proc_t> children;
        for (auto i = 0; i < CHILDREN; ++i)
            children.push_back(make_proc<child>(bar));
        // Perform 100 main syncs
        for (auto i = 0; i < 100; ++i)
        {
            make_par(children)();
            cout << "Child " << _idx << " syncing..." << endl;
            _main_bar();
        }
    }
};

int main(int argc, char **argv) noexcept
{
    // Select the concurrency model to use
    // using model = thread_model;
    // using model = thread_fiber_model;
    // using model = atomic_model;
    using model = atomic_fiber_model;
    // using model = fiber_model;

    // Create main barrier
    auto bar = model::make_bar(PARENTS);
    // Create vector of processes
    vector<proc_t> procs;
    for (auto i = 0; i < PARENTS; ++i)
        procs.push_back(make_proc<parent>(i, bar));
    // Run in parallel
    // TODO - run in parallel
    return 0;
}