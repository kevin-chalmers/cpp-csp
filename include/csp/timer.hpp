#pragma once

#include <thread>
#include <chrono>
#include "alternative.hpp"

namespace csp
{
    template<class CLOCK, class DURATION = typename CLOCK::duration, class TIME_POINT = typename CLOCK::time_point>
    class timer_internal : public guard_internal
    {
    private:
        TIME_POINT _time;

    public:
        timer_internal()
        {
        }

        timer_internal(const timer_internal&) = default;

        timer_internal(timer_internal&&) = default;

        ~timer_internal() = default;

        timer_internal& operator=(const timer_internal&) = default;

        timer_internal& operator=(timer_internal&) = default;

        inline TIME_POINT get_alarm() const noexcept { return _time; }

        inline void set_alarm(TIME_POINT &time_point) noexcept
        {
            _time = time_point;
        }

        bool enable(alt_internal* a) noexcept final
        {
            if (_timer - CLOCK::now() <= CLOCK::duration::zero())
                return true;
            set_timeout(a, _time);
            return false;
        }

        inline bool disable() noexcept final
        {
            return _time - CLOCK::now() <= CLOCK::duration::zero();
        }
    };

    template<class CLOCK, class DURATION = typename CLOCK::duration, class TIME_POINT = typename CLOCK::time_point>
    class timer : public guard
    {
    private:
        std::shared_ptr<timer_internal> _internal;

    public:
        timer()
        _internal(std::make_shared<timer_internal>())
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
            return _internal->get_alarm();
        }

        inline void set_alarm(const TIME_POINT &timepoint) const noexcept
        {
            _internal->set_alarm(timepoint);
        }

        inline void set_alarm(const DURATION &duration) const noexcept
        {
            _internal->set_alarm(CLOCK::now() + duration);
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