//
// Created by kevin on 15/04/16.
//

#ifndef CPP_CSP_TIMER_H
#define CPP_CSP_TIMER_H

#include <thread>
#include <chrono>
#include "guard.h"

namespace csp
{
    /*! \class timer
     * \brief A timer used as a guard for selection purposes.
     *
     * \author Kevin Chalmers
     *
     * \date 15/4/2016
     */
    class timer : public guard
    {
    private:
        /*! \class timer_internal
         * \brief Internal representation of a timer guard
         *
         * \author Kevin Chalmers
         *
         * \date 15/4/2016
         */
        class timer_internal : public guard::guard_internal
        {
        private:
            std::chrono::steady_clock::time_point _time; //<! The time to wait until

        public:
            /*!
             * \brief Creates an internal timer object
             */
            timer_internal() noexcept { }

            /*
             * \brief Destroys the timer object.
             */
            ~timer_internal() { }

            /*!
             * \brief Gets the current time to wait to
             *
             * \return The current timepoint to wait until
             */
            std::chrono::steady_clock::time_point get_alarm() const noexcept { return _time; }

            /*!
             * \brief Sets the time to wait to
             *
             * \param[in] timepoint The timepoint to wait until
             */
            void set_alarm(const std::chrono::steady_clock::time_point &timepoint) noexcept { _time = timepoint; }

            /*!
             * \brief Enables timer in a selection operation.
             *
             * \param[in] a The alt being used to select the guard.
             *
             * \return True if the guard is ready, false otherwise.
             */
            bool enable(const alt &a) noexcept override final
            {
                if (_time - std::chrono::steady_clock::now() <= std::chrono::steady_clock::duration::zero())
                    return true;
                set_timeout(a, _time);
                return false;
            }

            /*!
             * \brief Disables timer in a selection operation.
             *
             * \return True if the guard is ready, false otherwise.
             */
            bool disable() noexcept override final
            {
                return _time - std::chrono::steady_clock::now() <= std::chrono::steady_clock::duration::zero();
            }
        };

        std::shared_ptr<timer_internal> _internal = nullptr; //<! Pointer to the internal pointer representation.

    public:
        /*!
         * \brief Creates a new timer.
         */
        timer() noexcept : _internal(std::shared_ptr<timer_internal>(new timer_internal())), guard(nullptr)
        {
            this->guard::_internal = _internal;
        }

        /*!
         * \brief Copy constructor.  Copies the timer object.
         */
        timer(const timer &other) noexcept = default;

        /*!
         * \brief Move constructor.  Copies the timer object using move semantics.
         */
        timer(timer &&rhs) noexcept = default;

        /*!
         * \brief Copy assignment operator.  Copies the timer object.
         */
        timer& operator=(const timer &other) noexcept = default;

        /*!
         * \brief Move assignment operator.  Copies the timer object.
         */
        timer& operator=(timer &&rhs) noexcept = default;

        /*!
         * \brief Destroys the timer.
         */
        ~timer() noexcept { }

        /*!
         * \brief Reads the current time.
         *
         * \return The current time.
         */
        std::chrono::steady_clock::time_point read() const noexcept
        {
            return std::chrono::steady_clock::now();
        }

        /*!
         * \brief Operator overload to return the current time.
         *
         * \return The current time.
         */
        std::chrono::steady_clock::time_point operator()() const noexcept
        {
            return read();
        }

        /*!
         * \brief Gets the current time to wait to.
         *
         * \return Current alarm time.
         */
        std::chrono::steady_clock::time_point get_alarm() const noexcept { return _internal->get_alarm(); }

        /*!
         * \brief Sets the time to wait to.
         *
         * \param[in] timepoint The time to set as the alarm time.
         */
        void set_alarm(const std::chrono::steady_clock::time_point &timepoint) noexcept { _internal->set_alarm(timepoint); }

        /*!
         * \brief Sets the current time to wait for.
         *
         * \param[in] duration The duration to wait for.
         */
        void set_alarm(const std::chrono::steady_clock::duration &duration) noexcept { _internal->set_alarm(read() + duration); }

        /*!
         * \brief Waits until the given time point is reached.
         *
         * \param[in] timepoint The time to wait to.
         */
        void after(const std::chrono::steady_clock::time_point &timepoint) const noexcept
        {
            // Sleep until time point
            std::this_thread::sleep_until(timepoint);
        }

        /*!
         * \brief Operator overload to wait until the given time point
         *
         * \param[in] timepoint The time to wait to.
         */
        void operator()(const std::chrono::steady_clock::time_point &timepoint) const noexcept { after(timepoint); }

        /*!
         * \brief Waits for the given amount of time.
         *
         * \param[in] duration Time to wait for.
         */
        void sleep(const std::chrono::steady_clock::duration &duration) const noexcept
        {
            // Check that there is a duration
            if (duration > std::chrono::steady_clock::duration::zero())
                // Sleep for time
                std::this_thread::sleep_for(duration);
        }

        /*!
         * \brief Operator overload to wait for the given amount of time.
         *
         * \param[in] duration Time to wait for.
         */
        void operator()(const std::chrono::steady_clock::duration &duration) const noexcept
        {
            sleep(duration);
        }

        /*!
         * \brief Sets the alarm by adding the given duration to the current one.
         *
         * \param[in] duration The amount of time to wait for.
         *
         * \return The timer object.
         */
        timer& operator+(const std::chrono::steady_clock::duration &duration) noexcept
        {
            set_alarm(duration);
            return *this;
        }

        /*!
         * \brief Sets the alarm by adding the given duration to the current one.
         *
         * \param[in] duration The amount of time to wait for.
         *
         * \return The timer object.
         */
        timer& operator+=(const std::chrono::steady_clock::duration &duration) noexcept
        {
            set_alarm(duration);
            return *this;
        }

        /*!
         * \brief Sets the alarm by assigning the given timepoint as the alarm time.
         *
         * \param[in] timepoint The timepoint to set as the alarm time.
         *
         * \return The timer object.
         */
        timer& operator=(const std::chrono::steady_clock::time_point &timepoint) noexcept
        {
            set_alarm(timepoint);
            return *this;
        }
    };
}

#endif //CPP_CSP_TIMER_H
