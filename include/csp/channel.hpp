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
			std::vector<T> hold(1);
			bool reading = false;
			bool empty = true;
			size_t strength = 0;
		};

		std::shared_ptr<channel_data> _internal = nullptr;

	public:
		channel()
			: _internal(std::make_shared<channel_data>())
		{
		}

		void write(T value)
		{

		}

		T read()
		{
			return T();
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