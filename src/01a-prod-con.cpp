#include <iostream>
#include <thread>
#include <csp/csp.hpp>
#include <exception>

using namespace std;
using namespace csp;


class producer final : public process
{
private:
	chan_out<int> c;
public:
	explicit producer(chan_out<int> c)
	: c(move(c))
	{
	}

	void run() noexcept final
	{
		for (int i = 0; i < 100000; ++i)
			c(i);
	}
};

class consumer final : public process
{
private:
	guarded_chan_in<int> c1;
	guarded_chan_in<int> c2;
public:
	explicit consumer(guarded_chan_in<int> c1, guarded_chan_in<int> c2)
	: c1(move(c1)), c2(move(c2))
	{
	}

	void run() noexcept final
	{
		alternative alt(make_shared<thread_model::alt_type>(vector<guard>{c1, c2}));
		for (int i = 0; i < 200000; ++i)
        {
            size_t idx = alt();
            if (idx == 0)
            {
                cout << "Read " << c1() << " from channel 1" << endl;
            }
            else
            {
                cout << "Read " << c2() << " from channel 2" << endl;
            }
        }
	}
};

int main(int argc, char **argv) noexcept
{
	auto c1 = thread_model::make_one2one<int>();
	auto c2 = thread_model::make_one2one<int>();


	proc_t prod1 = make_proc<producer>(c1);
	proc_t prod2 = make_proc<producer>(c2);
	proc_t con = make_proc<consumer>(c1, c2);

	parallel<thread_model> p{ prod1, prod2, con };
	p.run();

	return 0;
}