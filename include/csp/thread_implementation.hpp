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

namespace csp
{
	namespace thread_implementation
	{
	    /*
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
	    */

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
/*
		struct mutex_guard
		{
		private:
			std::shared_ptr<std::mutex> mut = nullptr;
		public:
			mutex_guard()
				: mut(std::make_shared<std::mutex>())
			{
			}

			inline void lock() const noexcept
			{
				mut->lock();
			}

			inline void unlock() const noexcept
			{
				mut->unlock();
			}
		};
*/
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
/*
		class thread_type
		{
		public:
			std::function<void()> _process;
			std::shared_ptr<std::thread> _thread = nullptr;
			barrier_type _bar;
			barrier_type _park = barrier_type(2);
			bool _running = true;
		public:
			thread_type()
			{
			}

			thread_type(std::function<void()> &proc, barrier_type &bar)
				: _process(proc), _bar(bar)
			{
			}

			~thread_type()
			{
				thread_manager::remove_from_all_threads(_thread);
			}

			void reset(std::function<void()> &proc, barrier_type &bar) noexcept
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

		class parallel_type
		{
		private:
			struct par_data
			{
				std::mutex mut;
				std::vector<std::function<void()>> processes;
				std::vector<std::shared_ptr<thread_type>> threads;
				barrier_type bar = barrier_type(0);
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

		public:
			parallel_type(std::initializer_list<std::function<void()>> &&procs)
				: _internal(std::make_shared<par_data>())
			{
				_internal->processes = std::vector<std::function<void()>>(std::forward<std::initializer_list<std::function<void()>>>(procs));
			}

			parallel_type(std::vector<std::function<void()>> &procs)
				: _internal(std::make_shared<par_data>())
			{
				std::swap(_internal->processes, procs);
			}

			template<typename RanIt>
			parallel_type(RanIt begin, RanIt end)
				: _internal(std::make_shared<par_data>())
			{
				_internal->processes = std::vector<std::function<void()>>(begin, end);
			}

			void run() noexcept
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
								_internal->threads.push_back(std::make_shared<thread_type>(_internal->processes[i], _internal->bar));
								_internal->threads[i]->start();
							}
						}
						else
						{
							for (size_t i = _internal->threads.size() - 1; i >= _internal->processes.size() - 1; --i)
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
	*/
	}

	struct thread_model
    {
        template<typename T, bool POISONABLE = false>
        inline static channel<T, POISONABLE> make_chan() { return channel<T, POISONABLE>(std::make_shared<thread_implementation::channel_type<T, POISONABLE>>()); }

        inline static barrier make_bar(size_t enrolled = 0) { return barrier(std::make_shared<thread_implementation::barrier_type>(enrolled)); }
    };
}