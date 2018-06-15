#pragma once

namespace csp
{
    enum struct concurrency
    {
        THREAD_MODEL,
        ATOMIC_MODEL,
        FIBER_MODEL,
        THREAD_FIBER_MODEL,
        ATOMIC_FIBER_MODEL
    };
}