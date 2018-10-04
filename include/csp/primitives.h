#pragma once

#include "process.hpp"
#include "alternative.hpp"

namespace csp
{
    class skip : public process, public guard
    {
    private:
        class skip_internal : public guard_internal
        {
            bool enable(alt_internal*) { return true; }
            bool disable() { return true; }
        };
    public:
        skip()
        : _internal(std::make_shared<skip_internal>())
        {
        }

        skip(const skip&) = default;

        skip(skip&&) = default;

        ~skip() = default;

        skip& operator=(const skip&) = default;

        skip& operator=(skip&&) = default;

        void run() final
        {
        }
    };

    class stop : public process, public guard
    {
    private:
        class stop_internal : public guard_internal
        {
            bool enable(alt_internal*) { return false; }
            bool disable() { return false; }
        };
    public:
        stop()
        : _internal(std::make_shared<stop_internal>())
        {
        }

        stop(const stop&) = default;

        stop(stop&&) = default;

        ~stop() = default;

        stop& operator=(const stop&) = default;

        stop& operator=(stop&&) = default;

        void run() final
        {
            auto c = make_one2one<int>();
            c();
        }
    };

    guard operator&&(bool condition, guard& g)
    {
        if (condition)
            return g;
        else return stop();
    }
}