#pragma once

#include "channel.hpp"
#include "barrier.hpp"

namespace csp
{
    enum struct concurrency
    {
        THREAD_MODEL,
        THREAD_FIBER_MODEL,
        FIBER_MODEL
    };

    class concurrency_model_t
    {
    private:
        concurrency _model;
    public:
        concurrency_model_t(concurrency model)
        : _model(model)
        {
        }

        virtual ~concurrency_model_t() = default;

        //proc_t make_par()

        template<typename T, bool POISONABLE>
        channel<T, POISONABLE> make_chan() const noexcept;

        barrier make_bar(size_t enrolled = 0) const noexcept;
    };
}