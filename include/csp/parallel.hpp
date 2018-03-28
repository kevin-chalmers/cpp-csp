#pragma once

#include <memory>
#include <functional>
#include <thread>
#include <vector>
#include <set>
#include "barrier.hpp"
#include "process.hpp"

namespace csp
{
	class par_thread
	{
	private:
		std::function<void()> _process;
		std::shared_ptr<std::thread> _thread = nullptr;
		barrier _bar;
		barrier _park = barrier(2);
		bool _running = true;
	public:
		par_thread()
		{
		}

		par_thread(std::function<void()> &proc, barrier &bar)
			: _process(proc), _bar(bar)
		{
		}

		~par_thread();

		void reset(std::function<void()> proc, barrier &bar) noexcept
		{
			_process = proc;
			_bar = bar;
			_running = true;
		}

		void terminate() noexcept
		{
			_running = false;
			_park.sync();
		}

		void release() noexcept
		{
			_park.sync();
		}

		void run() noexcept;

		void start() noexcept;
	};

	class par : public process
	{
		friend class par_thread;
	private:

		static std::set<std::shared_ptr<std::thread>> _all_threads;

		static std::unique_ptr<std::mutex> _all_threads_lock;

		struct par_data
		{
			std::mutex mut;
			std::vector<std::function<void()>> processes;
			std::vector<std::shared_ptr<par_thread>> threads;
			barrier bar = barrier(0);
			bool processes_changed = true;
		};

		std::shared_ptr<par_data> _internal = nullptr;

		void release_all_threads() noexcept
		{
		}

		static void add_to_all_threads(std::shared_ptr<std::thread> thread) noexcept
		{
		}

		static void remove_from_all_threads(std::shared_ptr<std::thread> thread) noexcept
		{
		}

	public:
		par(std::initializer_list<std::function<void()>> &&procs)
		{
		}

		par(std::vector<std::function<void()>> &procs)
		{
		}

		template<typename RanIt>
		par(RanIt begin, RanIt end)
		{

		}

		~par()
		{
		}

		void run() noexcept final
		{

		}
	};

	par_thread::~par_thread()
	{
		par::remove_from_all_threads(_thread);
	}

	void par_thread::run() noexcept
	{
		while (_running)
		{
			_process();
			_bar.sync();
			_park.sync();
		}
	}

	void par_thread::start() noexcept
	{
		_thread = std::make_shared<std::thread>(&par_thread::run, this);
		par::add_to_all_threads(_thread);
		_running = true;
	}

	std::set<std::shared_ptr<std::thread>> par::_all_threads = std::set<std::shared_ptr<std::thread>>();

	std::unique_ptr<std::mutex> par::_all_threads_lock = std::make_unique<std::mutex>();
}