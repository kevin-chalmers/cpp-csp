#pragma once

#include <memory>
#include <vector>
#include <set>
#include "barrier.hpp"
#include "process.hpp"
#include "thread_implementation.hpp"

namespace csp
{
	template<typename IMPLEMENTATION = thread_implementation,
			 typename THREAD = IMPLEMENTATION::thread>
	class par_thread : public THREAD
	{
	public:
		par_thread()
		{
		}

		par_thread(std::function<void()> &proc, barrier<> &bar)
			: THREAD(proc, bar)
		{
		}

		~par_thread() = default;
	};

	template<typename IMPLEMENTATION = thread_implementation,
			 typename PARALLEL = IMPLEMENTATION::parallel,
			 typename THREAD = IMPLEMENTATION::thread>
	class par : public process<IMPLEMENTATION>, public PARALLEL
	{
	public:
		par(std::initializer_list<std::function<void()>> &&procs)
			: PARALLEL(std::forward<std::initializer_list<std::function<void()>>>(procs))
		{
		}

		par(std::vector<std::function<void()>> &procs)
			: PARALLEL(procs)
		{
		}

		template<typename RanIt>
		par(RanIt begin, RanIt end)
			: _internal(std::make_shared<par_data>())
		{
			static_assert(std::iterator_traits<RanIt>::value_type == typeid(std::function<void()>), "par only takes collections of void function objects");
		}

		~par()
		{
		}

		void run() noexcept final
		{
			PARALLEL::run();
		}
	};
}