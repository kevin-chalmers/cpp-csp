#pragma once

#include <memory>
#include <vector>
#include <mutex>
#include <condition_variable>
#include "thread_implementation.hpp"

namespace csp
{
	template<typename T, typename IMPLEMENTATION = thread_implementation::channel<T>>
	class channel : public IMPLEMENTATION
	{
	public:
		channel()
		{
		}

		channel(const channel<T, IMPLEMENTATION>&) = default;

		channel(channel<T, IMPLEMENTATION>&&) = default;

		~channel() = default;

		channel<T, IMPLEMENTATION>& operator=(const channel<T, IMPLEMENTATION>&) noexcept = default;

		channel<T, IMPLEMENTATION>& operator=(channel<T, IMPLEMENTATION>&&) noexcept = default;

		void operator()(T value) const
		{
			write(value);
		}

		T operator()() const
		{
			return std::move(read());
		}
	};
}