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
		for (int i = 0; i < 100000; ++i)
			cout << c.read() << endl;
	}
};

int main(int argc, char **argv) noexcept
{
	channel<int> c = thread_model::make_chan<int>();

	producer prod(c);
	consumer con(c);

	thread t1(prod);
	thread t2(con);

	t1.join();
	t2.join();

	return 0;
}