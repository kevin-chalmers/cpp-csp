#pragma once

#include <mutex>
#include <condition_variable>

namespace csp
{
	template<typename IMPLEMENTATION = thread_model::barrier>
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
	};
}