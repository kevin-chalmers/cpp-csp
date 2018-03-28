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
			std::lock<std::mutex> lock(_internal->mut);
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
				_internal->cond.notify_all()
			}
		}

		void reset(size_t enrolled) const noexcept
		{
		}
	};
}