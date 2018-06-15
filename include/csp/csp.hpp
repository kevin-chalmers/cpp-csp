#pragma once

#include "poison_exception.hpp"
#include "concurrency.hpp"
#include "channel.hpp"
#include "barrier.hpp"
#include "process.hpp"
#include "parallel.hpp"
#include "alternative.hpp"

#include "thread_implementation.hpp"

csp::proc_t csp::primitive_builder::make_par(concurrency model, const std::vector <proc_t> &procs) noexcept
{
    if (model == csp::concurrency::THREAD_MODEL)
        return csp::make_proc<csp::parallel<csp::thread_model>>(std::cref(procs));
}

csp::proc_t csp::primitive_builder::make_par(concurrency model, std::vector<proc_t> &&procs) noexcept
{
    if (model == csp::concurrency::THREAD_MODEL)
        return csp::make_proc<csp::parallel<csp::thread_model>>(std::move(procs));
}

template<typename T, bool POISONABLE = false>
csp::one2one_chan<T, POISONABLE> csp::primitive_builder::make_one2one(concurrency model) noexcept
{
    if (model == csp::concurrency::THREAD_MODEL)
        return csp::thread_model::make_one2one<T, POISONABLE>();
}

template<typename T, bool POISONABLE = false>
csp::one2any_chan<T, POISONABLE> csp::primitive_builder::make_one2any(concurrency model) noexcept
{
    if (model == csp::concurrency::THREAD_MODEL)
        return csp::thread_model::make_one2any<T, POISONABLE>();
}

template<typename T, bool POISONABLE = false>
csp::any2one_chan<T, POISONABLE> csp::primitive_builder::make_any2one(concurrency model) noexcept
{
    if (model == csp::concurrency::THREAD_MODEL)
        return csp::thread_model::make_any2one<T, POISONABLE>();
}

template<typename T, bool POISONABLE = false>
csp::any2any_chan<T, POISONABLE> csp::primitive_builder::make_any2any(concurrency model) noexcept
{
    if (model == csp::concurrency::THREAD_MODEL)
        return csp::thread_model::make_any2any<T, POISONABLE>();
}

csp::barrier csp::primitive_builder::make_bar(concurrency model, size_t enrolled) noexcept
{
    if (model == csp::concurrency::THREAD_MODEL)
        return csp::barrier(std::make_shared<csp::thread_implementation::barrier_type>(enrolled));
}

csp::alternative csp::primitive_builder::make_alt(concurrency model, const std::vector <guard> &guards) noexcept
{
    if (model == csp::concurrency::THREAD_MODEL)
        return csp::alternative(std::make_shared<csp::thread_model::alt_type>(std::cref(guards)));
}

csp::alternative csp::primitive_builder::make_alt(concurrency model, std::vector <guard> &&guards) noexcept
{
    if (model == csp::concurrency::THREAD_MODEL)
        return csp::alternative(std::make_shared<csp::thread_model::alt_type>(move(guards)));
}