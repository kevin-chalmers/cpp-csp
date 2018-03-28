#include <iostream>
#include <thread>
#include <csp/csp.hpp>

using namespace std;

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
	thread prod(producer);
	thread con(consumer);
	prod.join();
	con.join();
	return 0;
}