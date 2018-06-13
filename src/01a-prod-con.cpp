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
	chan_in<int> c;
public:
	explicit consumer(chan_in<int> c)
	: c(move(c))
	{
	}

	void run() noexcept final
	{
	    auto d = make_chan<int>();
	    auto b = make_bar();
		for (int i = 0; i < 100000; ++i)
			cout << c() << endl;
	}
};

int main(int argc, char **argv) noexcept
{
	auto c = thread_model::make_one2one<int>();
	alternative a(thread_model::alt_type());

	proc_t prod = make_proc<producer>(c);
	proc_t con = make_proc<consumer>(c);

	parallel<thread_model> p{ prod, con };
	p.run();

	return 0;
}