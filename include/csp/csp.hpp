#pragma once

#include "poison_exception.hpp"
#include "concurrency_model.hpp"
#include "channel.hpp"
#include "barrier.hpp"
#include "process.hpp"
#include "parallel.hpp"

#include "thread_implementation.hpp"

//template<typename T>
//using atomic_channel = nullptr_t;
//
//using shared = csp::thread::mutex_guard;
//
//using atomic_shared = nullptr_t;
//
//using barrier = csp::thread::barrier_type;
//
//using thread = csp::thread::thread_type;
//
//using parallel = csp::thread::parallel_type;

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