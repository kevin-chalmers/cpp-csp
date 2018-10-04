#pragma once

#include <thread>
#include <chrono>
#include "alternative.hpp"

namespace csp
{
    template<class CLOCK, class DURATION = typename CLOCK::duration, class TIME_POINT = typename CLOCK::time_point>
    class timer : public guard
    {
    public:
        timer()
        {

        }

        timer(const timer&) = default;

        timer(timer&&) = default;

        ~timer() = default;

        timer& operator=(const timer&) = default;

        timer& operator=(timer&&) = default;

        inline TIME_POINT read() const noexcept
        {
            return CLOCK::now();
        }

        inline TIME_POINT operator()() const noexcept()() const noexcept
        {
            return read();
        }

        inline TIME_POINT get_alarm() const noexcept
        {

        }

        inline void set_alarm(const TIME_POINT &timepoint) const noexcept
        {

        }

        inline void set_alarm(const DURATION &duration) const noexcept
        {

        }

        inline void after(const TIME_POINT &timepoint) const noexcept
        {
            // TODO: Need to work out sleeping for different models.
            // TODO: We can have a class with all the methods necessary if the model of concurrency stays the same (i.e. we don't have thread-fiber) since the methods will call the correct builder.
        }

        inline void operator()(const TIME_POINT &timepoint) const noexcept
        {
            after(timepoint);
        }

        inline void sleep(const DURATION &duration) const noexcept
        {
            // TODO: As after, need to define what sleep means in multi-model system.
        }

        inline void operator()(const DURATION &duration) const noexcept
        {
            sleep(DURATION);
        }

        timer<CLOCK>& operator+(const DURATION &duration) noexcept
        {
            set_alarm(duration);
            return *this;
        }

        timer<CLOCK>& operator+=(const DURATION &duration) noexcept
        {
            set_alarm(duration);
            return *this;
        }

        timer<CLOCK>& operator=(const TIME_POINT &timepoint) noexcept
        {
            set_alarm(timepoint);
            return *this;
        }
    };
}