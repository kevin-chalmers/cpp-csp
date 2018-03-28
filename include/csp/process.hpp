#pragma once

#include <functional>

namespace csp
{
	class process
	{
	public:
		virtual ~process() = default;

		virtual void run() noexcept = 0;

		void operator()() noexcept { this->run(); }
	};
}