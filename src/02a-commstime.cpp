#include <iostream>
#include <fstream>
#include <chrono>
#include <csp/csp.hpp>

using namespace std;
using namespace std::chrono;
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
        for (int i = 0; i < (1 << 24) - 1; ++i)
            b(a());
        a();
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
        for (int i = 0; i < (1 << 24); ++i)
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
        for (int i = 0; i < (1 << 24); ++i)
        {
            int tmp = b();
            c(tmp);
            d(tmp);
        }
    }
};

class par_delta final : public process
{
private:
    chan_in<int> b;
    chan_out<int> c;
    chan_out<int> d;
public:
    par_delta(chan_in<int> b, chan_out<int> c, chan_out<int> d)
    : b(move(b)), c(move(c)), d(move(d))
    {
    }

    void run() noexcept
    {
        for (int i = 0; i < (1 << 24); ++i)
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
    string type;
public:
    explicit consumer(chan_in<int> d, string type)
    : d(move(d)), type(type)
    {
    }

    void run() noexcept
    {
        ofstream res(string("commstime.") + type + string(".csv"));
        for (int i = 0; i < (1 << 24); i += (1 << 16))
        {
            auto start = system_clock::now();
            for (int j = 0; j < (1 << 16); ++j)
            {
                d();
            }
            auto total = (system_clock::now() - start).count();
            cout << i << endl;
            res << (static_cast<double>(total) / (1 << 16)) << ",";
        }
    }
};

int main(int argc, char **argv) noexcept
{
    {
        using conc = thread_model;
        auto a = primitive_builder::make_one2one<int>(conc::model_type);
        auto b = primitive_builder::make_one2one<int>(conc::model_type);
        auto c = primitive_builder::make_one2one<int>(conc::model_type);
        auto d = primitive_builder::make_one2one<int>(conc::model_type);

        parallel<conc>
        {
            make_proc<prefix>(a, b),
            make_proc<delta>(b, c, d),
            make_proc<successor>(c, a),
            make_proc<consumer>(d, string("thread"))
        }();
    }

    {
        using conc = thread_model;
        auto a = primitive_builder::make_one2one<int>(conc::model_type);
        auto b = primitive_builder::make_one2one<int>(conc::model_type);
        auto c = primitive_builder::make_one2one<int>(conc::model_type);
        auto d = primitive_builder::make_one2one<int>(conc::model_type);

        parallel<conc>
        {
            make_proc<prefix>(a, b),
            make_proc<par_delta>(b, c, d),
            make_proc<successor>(c, a),
            make_proc<consumer>(d, string("par.thread"))
        }();
    }

    {
        using conc = atomic_model;
        auto a = primitive_builder::make_one2one<int>(conc::model_type);
        auto b = primitive_builder::make_one2one<int>(conc::model_type);
        auto c = primitive_builder::make_one2one<int>(conc::model_type);
        auto d = primitive_builder::make_one2one<int>(conc::model_type);

        parallel<conc>
        {
            make_proc<prefix>(a, b),
            make_proc<delta>(b, c, d),
            make_proc<successor>(c, a),
            make_proc<consumer>(d, string("atomic"))
        }();
    }

    {
        using conc = atomic_model;
        auto a = primitive_builder::make_one2one<int>(conc::model_type);
        auto b = primitive_builder::make_one2one<int>(conc::model_type);
        auto c = primitive_builder::make_one2one<int>(conc::model_type);
        auto d = primitive_builder::make_one2one<int>(conc::model_type);

        parallel<conc>
        {
            make_proc<prefix>(a, b),
            make_proc<par_delta>(b, c, d),
            make_proc<successor>(c, a),
            make_proc<consumer>(d, string("par.atomic"))
        }();
    }

    {
        using conc = fiber_model;
        auto a = primitive_builder::make_one2one<int>(conc::model_type);
        auto b = primitive_builder::make_one2one<int>(conc::model_type);
        auto c = primitive_builder::make_one2one<int>(conc::model_type);
        auto d = primitive_builder::make_one2one<int>(conc::model_type);

        parallel<conc>
        {
            make_proc<prefix>(a, b),
            make_proc<delta>(b, c, d),
            make_proc<successor>(c, a),
            make_proc<consumer>(d, string("fiber"))
        }();
    }

    {
        using conc = fiber_model;
        auto a = primitive_builder::make_one2one<int>(conc::model_type);
        auto b = primitive_builder::make_one2one<int>(conc::model_type);
        auto c = primitive_builder::make_one2one<int>(conc::model_type);
        auto d = primitive_builder::make_one2one<int>(conc::model_type);

        parallel<conc>
        {
            make_proc<prefix>(a, b),
            make_proc<par_delta>(b, c, d),
            make_proc<successor>(c, a),
            make_proc<consumer>(d, string("par.fiber"))
        }();
    }

    return 0;
}

