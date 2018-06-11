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
	class parallel final : public process
	{
	    using PAR_TYPE = typename MODEL::par_type;
	private:
	    std::shared_ptr<PAR_TYPE> _internal = nullptr;
	public:
		explicit parallel(std::initializer_list<proc_t> &&procs)
		{
		    for (auto &p : procs)
		        p.set_model(MODEL::model_type);
		    _internal = std::make_shared<PAR_TYPE>(move(procs));
		}

		explicit parallel(std::vector<proc_t> &procs)
		{
		    for (auto &p : procs)
		        p.set_model(MODEL::model_type);
            _internal = std::make_shared<PAR_TYPE>(procs);
		}

		template<typename RanIt>
		parallel(RanIt begin, RanIt end)
		{
			static_assert(std::iterator_traits<RanIt>::value_type == typeid(proc_t), "par only takes collections of process objects");
		    for (auto p = begin; p != end; ++p)
		        p->set_model(MODEL::model_type);
            _internal = std::make_shared<PAR_TYPE>(begin, end);
		}

		~parallel()
		{
		}

		void run() noexcept
		{
			_internal->run();
		}
	};
}