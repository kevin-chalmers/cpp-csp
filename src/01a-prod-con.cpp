#include <iostream>
#include <thread>
#include <csp/csp.hpp>
#include <exception>

using namespace std;
using namespace csp;

class producer : public process
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

class consumer : public process
{
private:
	channel<int> c;
public:
	consumer(channel<int> c)
		: c(c)
	{
	}

	void run() noexcept final
	{
		for (int i = 0; i < 100000; ++i)
			cout << c() << endl;
	}
};

int main(int argc, char **argv) noexcept
{
	channel<int> c;

	par p{ producer(c), consumer(c) };
	p.run();

	return 0;
}