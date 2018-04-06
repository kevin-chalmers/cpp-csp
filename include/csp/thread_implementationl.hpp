#pragma once

#include <memory>
#include <vector>
#include <mutex>
#include <condition_variable>

namespace csp
{
	template<typename T>
	struct channel_data
	{
		std::mutex mut;
		std::condition_variable cond;
		std::vector<T> hold = std::vector<T>(0);
		bool reading = false;
		bool empty = true;
		size_t strength = 0;
	};

	template<typename T>
	class channel_type
	{
	private:
		std::shared_ptr<channel_data<T>> _internal = nullptr;
	public:
		channel_type()
			: _internal(std::make_shared<channel_data<T>>())
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

	struct thread_model
	{
		template<typename T>
		using channel = channel_type<T>;

		using process_data = nullptr_t;
		using channel_data = nullptr_t;
		using atomic_channel_data = nullptr_t;
	};
}