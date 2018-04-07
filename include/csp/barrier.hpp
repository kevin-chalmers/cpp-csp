#pragma once

#include <mutex>
#include <condition_variable>
#include "thread_implementation.hpp"

namespace csp
{
	template<typename IMPLEMENTATION = thread_implementation::barrier>
	class barrier : public IMPLEMENTATION
	{
	public:
		barrier()
		{
		}

		explicit barrier(size_t size)
			: IMPLEMENTATION(size)
		{
		}

		barrier(const barrier<IMPLEMENTATION>&) = default;

		barrier(barrier<IMPLEMENTATION>&&) = default;

		~barrier() = default;

		barrier<IMPLEMENTATION>& operator=(const barrier<IMPLEMENTATION>&) = default;

		barrier<IMPLEMENTATION>& operator=(barrier<IMPLEMENTATION>&&) = default;

		void operator()() const noexcept
		{
			sync();
		}
	};
}