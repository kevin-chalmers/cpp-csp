#pragma once

#include <functional>
#include <memory>
#include "concurrency_model.hpp"

namespace csp
{
	class process
	{
	private:
        concurrency_model_t _model = concurrency_model_t(concurrency::THREAD_MODEL);
	public:

		virtual ~process() = default;

        void set_model(concurrency model) noexcept { _model = concurrency_model_t(model); }

		template<typename T, bool POISONABLE = false>
		inline channel<T, POISONABLE> make_chan()
		{
			return _model.make_chan<T, POISONABLE>();
		}

		virtual void run() noexcept = 0;

		inline void operator()() noexcept { this->run(); }
	};
}