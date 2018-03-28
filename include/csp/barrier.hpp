#pragma once

namespace csp
{
	class barrier
	{
	public:
		barrier()
		{
		}

		barrier(size_t size)
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