#pragma once

#include <memory>
#include <vector>
#include <mutex>
#include <condition_variable>
#include "thread_implementation.hpp"

namespace csp
{
	struct unshared
	{
	protected:
		~unshared() = default;
	public:
		inline void lock() const noexcept { }

		inline void unlock() const noexcept { }
	};

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

	template<typename T, 
			 typename IMPLEMENTATION = thread_implementation,
			 typename CHANNEL_TYPE = IMPLEMENTATION::channel<T>,
		     typename SHARING_POLICY = unshared>
	class channel_input : public SHARING_POLICY
	{
	private:
		CHANNEL_TYPE _chan = nullptr;
	public:
		channel_input(CHANNEL_TYPE chan)
			: _chan(chan)
		{
		}

		T read() const
		{
			SHARING_POLICY::lock();
			auto to_return = _chan.read();
			SHARING_POLICY::unlock();
			return std::move(to_return);
		}
	};
}