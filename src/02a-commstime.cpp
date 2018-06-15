#include <iostream>
#include <csp/csp.hpp>

using namespace std;
using namespace csp;

class identity final : public process
{
private:
    chan_in<int> a;
    chan_out<int> b;
public:
    identity(chan_in<int> a, chan_out<int> b)
    : a(move(a)), b(move(b))
    {
    }

    void run() noexcept
    {
        while (true)
            b(a());
    }
};

class prefix final : public process
{
private:
    int pre = 0;
    chan_in<int> a;
    chan_out<int> b;
public:
    prefix(chan_in<int> a, chan_out<int> b, int pre = 0)
    : a(move(a)), b(move(b)), pre(pre)
    {
    }

    void run() noexcept
    {
        b(pre);
        identity(a, b)();
    }
};

class successor final : public process
{
private:
    chan_in<int> c;
    chan_out<int> a;
public:
    successor(chan_in<int> c, chan_out<int> a)
    : c(move(c)), a(move(a))
    {
    }

    void run() noexcept
    {
        while (true)
        {
            int tmp = c();
            a(++tmp);
        }
    }
};

class delta final : public process
{
private:
    chan_in<int> b;
    chan_out<int> c;
    chan_out<int> d;
public:
    delta(chan_in<int> b, chan_out<int> c, chan_out<int> d)
    : b(move(b)), c(move(c)), d(move(d))
    {
    }

    void run() noexcept
    {
        while (true)
        {
            int tmp = b();
            par_write({c, d}, tmp);
        }
    }
};

class consumer final : public process
{
private:
    chan_in<int> d;
public:
    explicit consumer(chan_in<int> d)
    : d(move(d))
    {
    }

    void run() noexcept
    {
        while (true)
            cout << d() << endl;
    }
};

int main(int argc, char **argv) noexcept
{
    concurrency conc = concurrency::THREAD_MODEL;
    auto a = primitive_builder::make_one2one<int>(conc);
    auto b = primitive_builder::make_one2one<int>(conc);
    auto c = primitive_builder::make_one2one<int>(conc);
    auto d = primitive_builder::make_one2one<int>(conc);

    parallel<thread_model>
    {
        make_proc<prefix>(a, b),
        make_proc<delta>(b, c, d),
        make_proc<successor>(c, a),
        make_proc<consumer>(d)
    }();
    return 0;
}

