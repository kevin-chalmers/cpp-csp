#include <iostream>
#include <thread>
#include <csp/csp.hpp>

using namespace std;
using namespace csp;

void producer()
{
	cout << "Producer started" << endl;
}

void consumer()
{
	cout << "Consumer started" << endl;
}

int main(int argc, char **argv) noexcept
{
	channel<int> c;

	thread prod(producer);
	thread con(consumer);
	prod.join();
	con.join();
	return 0;
}