#include <iostream>
#include <thread>
#include <csp/csp.hpp>
#include <exception>

using namespace std;
using namespace csp;


class producer final : public process
{
private:
	channel<int> c;
public:
	explicit producer(channel<int> c)
		: c(std::move(c))
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
	channel<int> c;
public:
	explicit consumer(channel<int> c)
		: c(std::move(c))
	{
	}

	void run() noexcept final
	{
	    auto d = make_chan<int>();
		for (int i = 0; i < 100000; ++i)
			cout << c.read() << endl;
	}
};

int main(int argc, char **argv) noexcept
{
	auto c = thread_model::make_chan<int>();
	auto b = thread_model::make_bar();

	producer prod(c);
	consumer con(c);
	prod.set_model(concurrency::THREAD_MODEL);
	con.set_model(concurrency::THREAD_MODEL);

	parallel<thread_model> p{prod, con};
	p.run();

	return 0;
}