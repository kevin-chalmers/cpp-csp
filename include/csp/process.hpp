#pragma once

#include <functional>

namespace csp
{
	template<typename IMPLEMENTATION = thread_implementation, 
			 typename PROCESS = IMPLEMENTATION::process_type>
	class process : public PROCESS
	{
	public:
		virtual ~process() = default;

		virtual void run() noexcept = 0;

		void operator()() noexcept { this->run(); }
	};
}