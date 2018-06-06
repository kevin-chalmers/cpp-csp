#pragma once

#include <memory>
#include <vector>
#include <set>
#include "barrier.hpp"
#include "process.hpp"
#include "concurrency_model.hpp"

namespace csp
{
	template<typename MODEL>
	class parallel : public process
	{
	    using PAR_TYPE = typename MODEL::par_type;
	private:
	    std::shared_ptr<PAR_TYPE> _internal = nullptr;
	public:
	    parallel()
        : _internal(std::make_shared<PAR_TYPE>())
        {
        }

		explicit parallel(std::initializer_list<process> &&procs)
        : _internal(std::make_shared<PAR_TYPE>(procs))
		{
		}

		explicit parallel(std::vector<process> &procs)
        : _internal(std::make_shared<PAR_TYPE>(procs))
		{
		}

		template<typename RanIt>
		parallel(RanIt begin, RanIt end)
        : _internal(std::make_shared<PAR_TYPE>(begin, end))
		{
			static_assert(std::iterator_traits<RanIt>::value_type == typeid(process), "par only takes collections of process objects");
		}

		~parallel()
		{
		}

		void run() noexcept final
		{
			_internal->run();
		}
	};
}