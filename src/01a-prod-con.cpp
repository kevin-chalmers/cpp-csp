#include <iostream>
#include <thread>
#include <csp/csp.hpp>
#include <exception>

using namespace std;
using namespace csp;

/*
class producer : public process<>
{
private:
	channel<int> c;
public:
	producer(channel<int> c)
		: c(c)
	{
	}

	void run() noexcept final
	{
		for (int i = 0; i < 100000; ++i)
			c(i);
	}
};

class consumer : public process<>
{
private:
	channel_input<int> c;
public:
	consumer(channel_input<int> c)
		: c(c)
	{
	}

	void run() noexcept final
	{
		for (int i = 0; i < 100000; ++i)
			cout << c.read() << endl;
	}
};
*/

void producer(channel<int> c)
{
    for (int i = 0; i < 10000; ++i)
        c(i);
}

void consumer(channel<int> c)
{
    for (int i = 0; i < 10000; ++i)
        cout << c() << endl;
}

int main(int argc, char **argv) noexcept
{
	channel<int> c = thread_model::make_chan<int>();

	thread prod(producer, c);
	thread con(consumer, c);

	prod.join();
	con.join();

	return 0;
}