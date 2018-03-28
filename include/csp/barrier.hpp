#pragma once

#include <mutex>
#include <condition_variable>

namespace csp
{
	class barrier
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
		barrier()
			: _internal(std::make_shared<barrier_data>())
		{
		}

		barrier(size_t size)
			: barrier(), _internal->enrolled(size), _internal->count_down(size)
		{
		}

		void sync() const noexcept
		{
		}

		void enroll() const noexcept
		{
		}

		void resign() const noexcept
		{
		}

		void reset(size_t enrolled) const noexcept
		{
		}
	};
}