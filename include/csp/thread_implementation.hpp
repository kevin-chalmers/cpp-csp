#pragma once

#include <memory>
#include <vector>
#include <set>
#include <functional>
#include <thread>
#include <mutex>
#include <condition_variable>

namespace csp
{
	namespace thread
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

		template<typename T>
		class channel_type
		{
		private:
			struct channel_data
			{
				std::mutex mut;
				std::condition_variable cond;
				std::vector<T> hold = std::vector<T>(0);
				bool reading = false;
				bool empty = true;
				size_t strength = 0;
			};

			std::shared_ptr<channel_data> _internal = nullptr;
		public:
			channel_type()
				: _internal(std::make_shared<channel_data>())
			{
			}

			void write(T value) const
			{
				std::unique_lock<std::mutex> lock(_internal->mut);
				if (_internal->strength > 0)
					throw poison_exception(_internal->strength);
				_internal->hold.push_back(std::move(value));
				if (_internal->empty)
				{
					_internal->empty = false;
				}
				else
				{
					_internal->empty = true;
					_internal->cond.notify_one();
				}
				_internal->cond.wait(lock);
				if (_internal->strength > 0)
					throw poison_exception(_internal->strength);
			}

			T read() const
			{
				std::unique_lock<std::mutex> lock(_internal->mut);
				if (_internal->strength > 0)
					throw poison_exception(_internal->strength);
				if (_internal->empty)
				{
					_internal->empty = false;
					_internal->cond.wait(lock);
				}
				else
					_internal->empty = true;
				auto to_return = std::move(_internal->hold[0]);
				_internal->hold.pop_back();
				_internal->cond.notify_one();
				if (_internal->strength > 0)
					throw poison_exception(_internal->strength);
				return std::move(to_return);
			}

			T start_read() const
			{
				return T();
			}

			void end_read() const
			{

			}

			bool enable() const noexcept
			{
				return false;
			}

			bool disable() const noexcept
			{
				return false;
			}

			bool pending() const noexcept
			{
				return false;
			}

			void reader_poison(size_t strength) const noexcept
			{

			}

			void writer_poison(size_t strength) const noexcept
			{

			}
		};

		class barrier_type
		{
		private:
			struct barrier_data
			{
				size_t enrolled = 0;
				size_t count_down = 0;
				std::mutex mut;
				std::condition_variable cond;
			};

			std::shared_ptr<barrier_data> _internal = nullptr;
		public:
			barrier_type()
				: _internal(std::make_shared<barrier_data>())
			{
			}

			explicit barrier_type(size_t size)
				: _internal(std::make_shared<barrier_data>())
			{
				_internal->enrolled = size;
				_internal->count_down = size;
			}

			void sync() const noexcept
			{
				std::unique_lock<std::mutex> lock(_internal->mut);
				--_internal->count_down;
				if (_internal->count_down > 0)
					_internal->cond.wait(lock);
				else
				{
					_internal->count_down = _internal->enrolled;
					_internal->cond.notify_all();
				}
			}

			void enroll() const noexcept
			{
				std::lock_guard<std::mutex> lock(_internal->mut);
				++_internal->enrolled;
				++_internal->count_down;
			}

			void resign() const noexcept
			{
				std::lock_guard<std::mutex> lock(_internal->mut);
				--_internal->enrolled;
				--_internal->count_down;
				if (_internal->count_down == 0)
				{
					_internal->count_down = _internal->enrolled;
					_internal->cond.notify_all();
				}
			}

			void reset(size_t enrolled) const noexcept
			{
				std::lock_guard<std::mutex> lock(_internal->mut);
				_internal->enrolled = enrolled;
				_internal->count_down = enrolled;
			}
		};

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
	}

	struct dummy
	{
	};

	struct thread_implementation
	{
		template<typename T>
		using channel = csp::thread::channel_type<T>;
		template<typename T>
		using atomic_channel = nullptr_t;
		using barrier = csp::thread::barrier_type;
		using process_type = dummy;
		using thread = csp::thread::thread_type;
		using parallel = dummy;
	};
}