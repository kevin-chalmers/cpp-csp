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
	public:
		std::function<void()> _process;
		std::shared_ptr<std::thread> _thread = nullptr;
		barrier _bar;
		barrier _park = barrier(2);
		bool _running = true;

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

			~par_data()
			{
				release_all_threads();
			}

			void release_all_threads() noexcept
			{
				std::lock_guard<std::mutex> lock(mut);
				for (auto &t : threads)
				{
					t->terminate();
					t->_thread->join();
				}
				processes_changed = true;
				threads.clear();
			}
		};

		std::shared_ptr<par_data> _internal = nullptr;

		static void add_to_all_threads(std::shared_ptr<std::thread> thread) noexcept
		{
			std::lock_guard<std::mutex> lock(*_all_threads_lock);
			_all_threads.emplace(thread);
		}

		static void remove_from_all_threads(std::shared_ptr<std::thread> thread) noexcept
		{
			std::lock_guard<std::mutex> lock(*_all_threads_lock);
			_all_threads.erase(thread);
		}

	public:
		par(std::initializer_list<std::function<void()>> &&procs)
			: _internal(std::make_shared<par_data>())
		{
			_internal->processes = std::vector<std::function<void()>>(std::forward<std:initializer_list<std::function<void()>>(procs));
		}

		par(std::vector<std::function<void()>> &procs)
			: _internal(std::make_shared<par_data>())
		{
			std::swap(_internal->processes, procs);
		}

		template<typename RanIt>
		par(RanIt begin, RanIt end)
			: _internal(std::make_shared<par_data>())
		{
			static_assert(std::iterator_traits<RanIt>::value_type == typeid(std::function<void()>), "par only takes collections of void function objects");
			_internal->processes = std::vector<std::function<void()>>(begin, end);
		}

		~par()
		{
		}

		void run() noexcept final
		{
			bool empty_run = true;
			std::function<void()> my_process;
			std::lock_guard<std::mutex> lock(_internal->mut);
			if (_internal->processes.size() > 0)
			{
				empty_run = false;
				my_process = _internal->processes[_internal->processes.size() - 1];
				if (_internal->processes_changed)
				{
					_internal->bar.reset(_internal->processes.size());
					if (_internal->threads.size() < _internal->processes.size() - 1)
					{
						for (size_t i = 0; i < _internal->threads.size(); ++i)
						{
							_internal->threads[i]->reset(_internal->processes[i], _internal->bar);
							_internal->threads[i]->release();
						}
						for (size_t i = _internal->threads.size(); i < _internal->processes.size() - 1; ++i)
						{
							_internal->threads.push_back(std::make_shared<par_thread>(_internal->processes[i], _internal->bar));
							_internal->threads[i]->start();
						}
					}
					else
					{
						for (size_t i = _internal->threads.size() - 1; i = > _internal->processes.size() - 1; --i)
						{
							_internal->threads[i]->_running = false;
							_internal->threads[i]->release();
						}
						_internal->threads.resize(_internal->processes.size() - 1);
						for (size_t i = 0; i < _internal->processes.size() - 1; ++i)
						{
							_internal->threads[i]->reset(_internal->processes[i], _internal->bar);
							_internal->threads[i]->release();
						}
					}
					_internal->processes_changed = false;
				}
				else
				{
					for (auto &t : _internal->threads)
						t->release();
				}
			}
			if (!empty_run)
			{
				my_process();
				_internal->bar.sync();
			}
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