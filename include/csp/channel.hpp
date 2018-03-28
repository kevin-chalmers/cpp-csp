#pragma once

namespace csp
{
	template<typename T>
	class channel
	{
	public:
		void write(T value)
		{

		}

		T read()
		{
			return T();
		}

		T start_read()
		{
			return T();
		}

		void end_read()
		{

		}

		bool enable() noexcept
		{
			return false;
		}

		bool disable() noexcept
		{
			return false;
		}

		bool pending() const noexcept
		{
			return false;
		}

		void reader_poison(size_t strength) noexcept
		{

		}

		void writer_poison(size_t strength) noexcept
		{

		}
	};
}