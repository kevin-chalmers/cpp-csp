#pragma once

#include <memory>
#include <vector>
#include <mutex>
#include <condition_variable>

namespace csp
{
	template<typename T>
	class channel
	{
	private:
		struct channel_data
		{
			std::mutex mut;
			std::condition_variable cond;
			std::vector<T> hold;
			bool reading = false;
			bool empty = true;
			size_t strength = 0;
		};

		std::shared_ptr<channel_data> _internal = nullptr;

	public:
		channel()
			: _internal(std::make_shared<channel_data>())
		{
			_internal->hold.resize(0);
		}

		void write(T value)
		{
			std::unique_lock<std::mutex> lock(_internal->mut);
			if (_internal->strength > 0)
				throw poison_exception(_strength);
			_hold.push_back(std::move(value));
			if (_internal->_empty)
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
				throw poison_exception(_strength);
		}

		T read()
		{
			std::unique_lock<std::mutex> lock(_mut);
			if (_internal->strength > 0)
				throw poison_exception(_strength);
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

		bool pending() const noexcept
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
}