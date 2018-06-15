#include <iostream>
#include <csp/csp.hpp>

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

	void run() noexcept
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

	void run() noexcept
	{
		for (int i = 0; i < 100000; ++i)
        {
            cout << c() << endl;
        }
	}
};

int main(int argc, char **argv) noexcept
{
    concurrency conc = concurrency::THREAD_MODEL;
	auto c = primitive_builder::make_one2one<int>(conc);

	parallel<thread_model>
    {
        make_proc<producer>(c),
        make_proc<consumer>(c)
    }();

	return 0;
}