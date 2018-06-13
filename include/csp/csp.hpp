#pragma once

#include "poison_exception.hpp"
#include "concurrency_model.hpp"
#include "channel.hpp"
#include "barrier.hpp"
#include "process.hpp"
#include "parallel.hpp"
#include "alternative.hpp"

#include "thread_implementation.hpp"

template<typename T, bool POISONABLE>
csp::channel<T, POISONABLE> csp::concurrency_model_t::make_chan() const noexcept
{
    if (_model == concurrency::THREAD_MODEL)
        return channel<T, POISONABLE>(std::make_shared<csp::thread_implementation::channel_type<T, POISONABLE>>());
}

csp::barrier csp::concurrency_model_t::make_bar(size_t enrolled) const noexcept
{
    if (_model == concurrency::THREAD_MODEL)
        return barrier(std::make_shared<csp::thread_implementation::barrier_type>(enrolled));
}