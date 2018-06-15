#pragma once

#include "poison_exception.hpp"
#include "concurrency.hpp"
#include "channel.hpp"
#include "barrier.hpp"
#include "process.hpp"
#include "parallel.hpp"
#include "alternative.hpp"

#include "thread_implementation.hpp"
#include "atomic_implementation.hpp"
#include "fiber_implementation.hpp"

csp::proc_t csp::primitive_builder::make_par(concurrency model, const std::vector <proc_t> &procs) noexcept
{
    if (model == csp::concurrency::THREAD_MODEL)
        return csp::make_proc<csp::parallel<csp::thread_model>>(std::cref(procs));
    else if (model == csp::concurrency::ATOMIC_MODEL)
        return csp::make_proc<csp::parallel<csp::atomic_model>>(std::cref(procs));
    else if (model == csp::concurrency::FIBER_MODEL)
        return csp::make_proc<csp::parallel<csp::fiber_model>>(std::cref(procs));
}

csp::proc_t csp::primitive_builder::make_par(concurrency model, std::vector<proc_t> &&procs) noexcept
{
    if (model == csp::concurrency::THREAD_MODEL)
        return csp::make_proc<csp::parallel<csp::thread_model>>(std::move(procs));
    else if (model == csp::concurrency::ATOMIC_MODEL)
        return csp::make_proc<csp::parallel<csp::atomic_model>>(std::move(procs));
    else if (model == csp::concurrency::FIBER_MODEL)
        return csp::make_proc<csp::parallel<csp::fiber_model>>(std::move(procs));
}

template<typename T, bool POISONABLE = false>
csp::one2one_chan<T, POISONABLE> csp::primitive_builder::make_one2one(concurrency model) noexcept
{
    if (model == csp::concurrency::THREAD_MODEL)
        return csp::thread_model::make_one2one<T, POISONABLE>();
    else if (model == csp::concurrency::ATOMIC_MODEL)
        return csp::atomic_model::make_one2one<T, POISONABLE>();
    else if (model == csp::concurrency::FIBER_MODEL)
        return csp::fiber_model::make_one2one<T, POISONABLE>();
}

template<typename T, bool POISONABLE = false>
csp::one2any_chan<T, POISONABLE> csp::primitive_builder::make_one2any(concurrency model) noexcept
{
    if (model == csp::concurrency::THREAD_MODEL)
        return csp::thread_model::make_one2any<T, POISONABLE>();
    else if (model == csp::concurrency::ATOMIC_MODEL)
        return csp::atomic_model::make_one2any<T, POISONABLE>();
    else if (model == csp::concurrency::FIBER_MODEL)
        return csp::fiber_model::make_one2any<T, POISONABLE>();
}

template<typename T, bool POISONABLE = false>
csp::any2one_chan<T, POISONABLE> csp::primitive_builder::make_any2one(concurrency model) noexcept
{
    if (model == csp::concurrency::THREAD_MODEL)
        return csp::thread_model::make_any2one<T, POISONABLE>();
    else if (model == csp::concurrency::ATOMIC_MODEL)
        return csp::atomic_model::make_any2one<T, POISONABLE>();
    else if (model == csp::concurrency::FIBER_MODEL)
        return csp::atomic_model::make_any2one<T, POISONABLE>();
}

template<typename T, bool POISONABLE = false>
csp::any2any_chan<T, POISONABLE> csp::primitive_builder::make_any2any(concurrency model) noexcept
{
    if (model == csp::concurrency::THREAD_MODEL)
        return csp::thread_model::make_any2any<T, POISONABLE>();
    else if (model == csp::concurrency::ATOMIC_MODEL)
        return csp::atomic_model::make_any2any<T, POISONABLE>();
    else if (model == csp::concurrency::FIBER_MODEL)
        return csp::fiber_model::make_any2any<T, POISONABLE>();
}

csp::barrier csp::primitive_builder::make_bar(concurrency model, size_t enrolled) noexcept
{
    if (model == csp::concurrency::THREAD_MODEL)
        return csp::thread_model::make_bar(enrolled);
    else if (model == csp::concurrency::ATOMIC_MODEL)
        return csp::atomic_model::make_bar(enrolled);
    else if (model == csp::concurrency::FIBER_MODEL)
        return csp::fiber_model::make_bar(enrolled);
}

csp::alternative csp::primitive_builder::make_alt(concurrency model, const std::vector <guard> &guards) noexcept
{
    if (model == csp::concurrency::THREAD_MODEL)
        return csp::alternative(std::make_shared<csp::thread_model::alt_type>(std::cref(guards)));
    else if (model == csp::concurrency::ATOMIC_MODEL)
        return csp::alternative(std::make_shared<csp::atomic_model::alt_type>(std::cref(guards)));
    else if (model == csp::concurrency::FIBER_MODEL)
        return csp::alternative(std::make_shared<csp::fiber_model::alt_type>(std::cref(guards)));
}

csp::alternative csp::primitive_builder::make_alt(concurrency model, std::vector <guard> &&guards) noexcept
{
    if (model == csp::concurrency::THREAD_MODEL)
        return csp::alternative(std::make_shared<csp::thread_model::alt_type>(std::move(guards)));
    else if (model == csp::concurrency::ATOMIC_MODEL)
        return csp::alternative(std::make_shared<csp::atomic_model::alt_type>(std::move(guards)));
    else if (model == csp::concurrency::FIBER_MODEL)
        return csp::alternative(std::make_shared<csp::fiber_model::alt_type>(std::move(guards)));
}

namespace csp
{
    struct thread_fiber_model
    {
        static constexpr csp::concurrency model_type = csp::concurrency::FIBER_MODEL;
        using par_type = thread_model::par_type;
    };

    struct atomic_fiber_model
    {
        static constexpr csp::concurrency model_type = csp::concurrency::FIBER_MODEL;
        using par_type = thread_model::par_type;
    };
}