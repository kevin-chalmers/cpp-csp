#pragma once

#include <memory>
#include <vector>
#include <set>
#include <functional>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <cassert>
#include "channel.hpp"
#include "barrier.hpp"
#include "parallel.hpp"

namespace csp
{
	namespace thread_implementation
	{
		class thread_manager
		{
		private:
			static std::set<std::shared_ptr<std::thread>> _all_threads;
			static std::unique_ptr<std::mutex> _all_threads_lock;
		public:
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
		};

		std::set<std::shared_ptr<std::thread>> thread_manager::_all_threads = std::set<std::shared_ptr<std::thread>>();
		std::unique_ptr<std::mutex> thread_manager::_all_threads_lock = std::make_unique<std::mutex>();

		template<typename T, bool POISONABLE = false>
		class channel_type final : public csp::channel_internal<T, POISONABLE>
		{
		private:
            std::mutex _mut;
            std::condition_variable _cond;
            std::vector<T> _hold = std::vector<T>(0);
            bool _reading = false;
            bool _empty = true;
            size_t _strength = 0;
		public:
			channel_type()
			{
			}

			void write(T value)
			{
				std::unique_lock<std::mutex> lock(_mut);
				if (_strength > 0)
					throw poison_exception(_strength);
				_hold.push_back(std::move(value));
				if (_empty)
				{
					_empty = false;
				}
				else
				{
					_empty = true;
					_cond.notify_one();
				}
				_cond.wait(lock);
				if (_strength > 0)
					throw poison_exception(_strength);
			}

            void write(T&& value)
            {
                std::unique_lock<std::mutex> lock(_mut);
                if (_strength > 0)
                    throw poison_exception(_strength);
                _hold.push_back(value);
                if (_empty)
                {
                    _empty = false;
                }
                else
                {
                    _empty = true;
                    _cond.notify_one();
                }
                _cond.wait(lock);
                if (_strength > 0)
                    throw poison_exception(_strength);
            }

			T read()
			{
				std::unique_lock<std::mutex> lock(_mut);
				if (_strength > 0)
					throw poison_exception(_strength);
				if (_empty)
				{
					_empty = false;
					_cond.wait(lock);
				}
				else
					_empty = true;
				auto to_return = std::move(_hold[0]);
				_hold.pop_back();
				_cond.notify_one();
				if (_strength > 0)
					throw poison_exception(_strength);
				return std::move(to_return);
			}

			T start_read()
			{
				return T();
			}

			void end_read()
			{

			}

			bool enable() noexcept
			{
				return false;
			}

			bool disable() noexcept
			{
				return false;
			}

			bool pending() noexcept
			{
				return false;
			}

			void reader_poison(size_t strength) noexcept
			{

			}

			void writer_poison(size_t strength) noexcept
			{

			}
		};

		class barrier_type : public barrier_internal
		{
		private:
            size_t _enrolled = 0;
            size_t _count_down = 0;
            std::mutex _mut;
            std::condition_variable _cond;

		public:
			barrier_type()
			{
			}

			explicit barrier_type(size_t size)
			{
				_enrolled = size;
				_count_down = size;
			}

			void sync() noexcept
			{
				std::unique_lock<std::mutex> lock(_mut);
				--_count_down;
				if (_count_down > 0)
					_cond.wait(lock);
				else
				{
					_count_down = _enrolled;
					_cond.notify_all();
				}
			}

			void enroll() noexcept
			{
				std::lock_guard<std::mutex> lock(_mut);
				++_enrolled;
				++_count_down;
			}

			void resign() noexcept
			{
				std::lock_guard<std::mutex> lock(_mut);
				--_enrolled;
				--_count_down;
				if (_count_down == 0)
				{
					_count_down = _enrolled;
					_cond.notify_all();
				}
			}

			void reset(size_t enrolled) noexcept
			{
			    assert(_enrolled == _count_down);
				std::lock_guard<std::mutex> lock(_mut);
				_enrolled = enrolled;
				_count_down = enrolled;
			}
		};

		class thread_type
		{
		public:
			std::function<void()> _process;
			std::shared_ptr<std::thread> _thread = nullptr;
			barrier _bar;
			barrier _park = barrier(std::make_shared<barrier_type>(2));
			bool _running = true;
		public:
			thread_type(std::function<void()> &proc, barrier &bar)
            : _process(proc), _bar(bar)
			{
			}

			~thread_type()
			{
				thread_manager::remove_from_all_threads(_thread);
			}

			void reset(std::function<void()> &proc, barrier &bar) noexcept
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

			void run() noexcept
			{
				while (_running)
				{
					_process();
					_bar.sync();
					_park.sync();
				}
			}

			void start() noexcept
			{
				_thread = std::make_shared<std::thread>(&thread_type::run, this);
				thread_manager::add_to_all_threads(_thread);
				_running = true;
			}
		};

		class parallel_type : public csp::parallel_internal
		{
		private:
			std::mutex _mut;
			std::vector<std::function<void()>> _processes;
			std::vector<std::shared_ptr<thread_type>> _threads;
			barrier _bar = barrier(std::make_shared<barrier_type>(0));
			bool _processes_changed = true;

			void release_all_threads() noexcept
			{
				std::lock_guard<std::mutex> lock(_mut);
				for (auto &t : _threads)
				{
					t->terminate();
					t->_thread->join();
				}
				_processes_changed = true;
				_threads.clear();
			}

		public:
		    parallel_type()
            {
            }

			explicit parallel_type(std::initializer_list<std::function<void()>> &&procs)
			{
				_processes = std::vector<std::function<void()>>(std::forward<std::initializer_list<std::function<void()>>>(procs));
			}

			explicit parallel_type(std::initializer_list<std::function<void()>> &procs)
            {
                _processes = std::vector<std::function<void()>>(std::forward<std::initializer_list<std::function<void()>>>(procs));
            }

			explicit parallel_type(std::vector<std::function<void()>> &procs)
			{
				std::swap(_processes, procs);
			}

			template<typename RanIt>
			parallel_type(RanIt begin, RanIt end)
			{
				static_assert(std::iterator_traits<RanIt>::value_type == typeid(std::function<void()>), "par only takes collections of void function objects");
				_processes = std::vector<std::function<void()>>(begin, end);
			}

			~parallel_type()
            {
                release_all_threads();
            }

			void run() noexcept
			{
				bool empty_run = true;
				std::function<void()> my_process;
				std::lock_guard<std::mutex> lock(_mut);
				if (_processes.size() > 0)
				{
					empty_run = false;
					my_process = _processes[_processes.size() - 1];
					if (_processes_changed)
					{
						_bar.reset(_processes.size());
						if (_threads.size() < _processes.size() - 1)
						{
							for (size_t i = 0; i < _threads.size(); ++i)
							{
								_threads[i]->reset(_processes[i], _bar);
								_threads[i]->release();
							}
							for (size_t i = _threads.size(); i < _processes.size() - 1; ++i)
							{
								_threads.push_back(std::make_shared<thread_type>(_processes[i], _bar));
								_threads[i]->start();
							}
						}
						else
						{
							for (size_t i = _threads.size() - 1; i >= _processes.size() - 1; --i)
							{
								_threads[i]->_running = false;
								_threads[i]->release();
							}
							_threads.resize(_processes.size() - 1);
							for (size_t i = 0; i < _processes.size() - 1; ++i)
							{
								_threads[i]->reset(_processes[i], _bar);
								_threads[i]->release();
							}
						}
						_processes_changed = false;
					}
					else
					{
						for (auto &t : _threads)
							t->release();
					}
				}
				if (!empty_run)
				{
					my_process();
					_bar.sync();
				}
			}
		};
	}

	struct thread_model
    {
        static constexpr concurrency model_type = concurrency::THREAD_MODEL;

        using par_type = thread_implementation::parallel_type;

        template<typename T, bool POISONABLE = false>
        inline static channel<T, POISONABLE> make_chan() { return channel<T, POISONABLE>(std::make_shared<thread_implementation::channel_type<T, POISONABLE>>()); }

        inline static barrier make_bar(size_t enrolled = 0) { return barrier(std::make_shared<thread_implementation::barrier_type>(enrolled)); }
    };
}