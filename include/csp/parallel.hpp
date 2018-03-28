#pragma once

#include <functional>
#include <thread>
#include "process.hpp"

namespace csp
{
	class par : public process
	{
	private:
		void release_all_threads() noexcept
		{
		}

		static void add_to_all_threads(std::shared_ptr<std::thread> thread) noexcept
		{
		}

		static void remove_from_all_threads(std::shared_ptr<std::thread> thread) noexcept
		{
		}

	public:
		par(std::initializer_list<std::function<void()>> &&procs)
		{
		}

		par(std::vector<std::function<void()>> &procs)
		{
		}

		template<typename RanIt>
		par(RanIt begin, RanIt end)
		{

		}

		~par()
		{
		}

		void run() noexcept final
		{

		}
	};
}