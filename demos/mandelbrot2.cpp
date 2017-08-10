//
// Created by kevin on 04/08/16.
//

#include <iostream>
#include <string>
#include <vector>
#include <array>
#include <chrono>
#include <fstream>
#include <cmath>
#include "../csp/csp.h"

using namespace std;
using namespace std::chrono;
using namespace csp;

constexpr unsigned int MAX_ITERATIONS = 255;

unsigned int DIM = 256;

constexpr double xmin = -2.1;
constexpr double xmax = 1.0;
constexpr double ymin = -1.3;
constexpr double ymax = 1.3;

double integral_x = (xmax - xmin) / static_cast<double>(DIM);
double integral_y = (ymax - ymin) / static_cast<double>(DIM);

int NUM_WORKERS = 1;

struct mandelbrot_packet
{
    int line = 0;
    vector<double> data;
};

void mandelbrot(chan_in<int> in, chan_out<mandelbrot_packet> out) noexcept
{
    int line = in();

    while (line != -1)
    {
        double x, y, x1, y1, xx = 0.0;
        unsigned int loop_count = 0;

        mandelbrot_packet packet;
        packet.line = line;
        packet.data = vector<double>(DIM);

        y = ymin + (line * integral_y);
        x = xmin;
        for (unsigned int x_coord = 0; x_coord < DIM; ++x_coord)
        {
            x1 = 0.0, y1 = 0.0;
            loop_count = 0;
            while (loop_count < MAX_ITERATIONS && sqrt(pow(x1, 2.0) + pow(y1, 2.0)) < 2.0)
            {
                ++loop_count;
                xx = pow(x1, 2.0) - pow(y1, 2.0) + x;
                y1 = 2 * x1 * y1 + y;
                x1 = xx;
            }
            auto val = static_cast<double>(loop_count) / static_cast<double>(MAX_ITERATIONS);
            packet.data[x_coord] = val;
            x += integral_x;
        }
        out(move(packet));
        line = in();
    }
}

void producer(chan_out<int> out, int lines, int num_workers) noexcept
{
    for (int i = 0; i < lines; ++i)
        out(i);
    for (int i = 0; i < num_workers; ++i)
        out(-1);
}

void consumer(chan_in<mandelbrot_packet> in, int lines) noexcept
{
    vector<vector<double>> results(lines);
    for (int i = 0; i < lines; ++i)
    {
        auto packet = in();
        results[packet.line] = std::move(packet.data);
    }
}

int main(int argc, char **argv)
{
    if (argc == 3)
    {
        DIM = stoi(argv[1]);
        NUM_WORKERS = stoi(argv[2]);
    }

    one2any_chan<int> lines;
    any2one_chan<mandelbrot_packet> data;

    vector<function<void()>> workers;
    for (int i = 0; i < NUM_WORKERS; ++i)
        workers.push_back(make_proc(mandelbrot, lines, data));

    array<unsigned long long, 100> res;
    for (unsigned int i = 0; i < 100; ++i)
    {
        auto start = system_clock::now();
        par
        {
            make_proc(producer, lines, DIM, NUM_WORKERS),
            par(workers),
            make_proc(consumer, data, DIM)
        }();
        auto stop = system_clock::now();
        res[i] = duration_cast<nanoseconds>(stop - start).count();
        cout << i << ": " << res[i] << endl;
    }
    ofstream results("mandelbrot_move_" + to_string(NUM_WORKERS) + "_" + to_string(DIM) + ".csv");
    for (unsigned int i = 0; i < 100; ++i)
        results << res[i] << ",";
    results.close();
    return 0;
}

