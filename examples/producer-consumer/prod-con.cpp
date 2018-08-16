#include <iostream>
#include <string>
#include <csp/csp.hpp>

using namespace std;
using namespace csp;

class producer : public process
{
    chan_out<string> _out;

public:
    producer(chan_out<string> out)
    : _out(move(out))
    {
    }

    void run() noexcept final
    {
        string str;
        while (str != "exit")
        {
            cout << "Enter data to send: ";
            cin >> str;
            _out(str);
        }
    }
};

class consumer : public process
{
    chan_in<string> _in;

public:
    consumer(chan_in<string> in)
    : _in(move(in))
    {
    }

    void run() noexcept final
    {
        string str;
        while (str != "exit")
        {
            str = _in();
            cout << str << endl;
        }
    }
};

int main(int argc, char **argv) noexcept
{
    // Select the concurrency model to use
    using model = thread_model;
	auto c = model::make_one2one<string>();

	parallel<model>
    {
        make_proc<producer>(c),
        make_proc<consumer>(c)
    }();

	return 0;
}