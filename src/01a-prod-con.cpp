#include <iostream>
#include <thread>
#include <csp/csp.hpp>
#include <exception>

using namespace std;
using namespace csp;

void producer(channel<int> c)
{
	for (int i = 0; i < 10000000; ++i)
		c.write(i);
}

void consumer(channel<int> c)
{
	for (int i = 0; i < 10000000; ++i)
	{
		auto n = c.read();
		if (n != i)
			throw runtime_error("Error in the write");
	}
}

int main(int argc, char **argv) noexcept
{
	channel<int> c;

	thread prod(producer, c);
	thread con(consumer, c);
	prod.join();
	con.join();
	return 0;
}