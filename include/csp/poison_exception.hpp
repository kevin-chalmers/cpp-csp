#pragma once

#include <exception>
#include <sstream>

namespace csp
{
	class poison_exception : public std::exception
	{
	private:
		size_t _strength;
	public:
		poison_exception(size_t strength) noexcept
			: _strength(strength)
		{
		}

		const char* what() const noexcept final
		{
			return "Channel has been poisoned";
		}

		size_t get_strength() const noexcept
		{
			return _strength;
		}
	};
}