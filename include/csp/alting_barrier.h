//
// Created by kevin on 15/04/16.
//

#ifndef CPP_CSP_ALTING_BARRIER_H
#define CPP_CSP_ALTING_BARRIER_H

#include <vector>
#include <algorithm>
#include <thread>
#include <mutex>
#include "alt.h"
#include "barrier.h"
#include "timer.h"

namespace csp
{
    /*! \class alting_barrier
     * \brief An event object that can be used as a multiway synchronization.
     *
     * \author Kevin Chalmers
     *
     * \date 15/4/2016
     */
    class alting_barrier : public barrier, public guard
    {
    private:
        /*! \class alting_barrier_internal
         * \brief Internal representation of a alting barrier
         *
         * \author Kevin Chalmers
         *
         * \date 15/4/2016
         */
        class alting_barrier_internal : public guard::guard_internal, public multiway_sync, public barrier::barrier_internal, public std::enable_shared_from_this<alting_barrier_internal>
        {
        public:
            /*!
             * \class alting_barrier_base
             * \brief Internal control object of an alting barrier
             *
             * \author Kevin Chalmers
             *
             * \date 15/4/2016
             */
            class alting_barrier_base
            {
            public:
                std::mutex _mut; //<! Lock to control access to the alting barrier base

                std::vector<std::shared_ptr<alting_barrier_internal>> _front_ends; //<! Vector of front ends associated with the alting barrier base

                unsigned int _enrolled = 0; //<! Number of processes enrolled with the alting barrier

                unsigned int _countdown = 0; //<! Number of processes yet to sync on the barrier

                /*!
                 * \brief Creates a new alting barrier front end associated with the base.
                 *
                 * \param[in] ab The alting barrier being created
                 */
                void expand(std::shared_ptr<alting_barrier_internal> ab) noexcept
                {
                    // Increment enrolled and countdown
                    ++_enrolled;
                    ++_countdown;

                    // Add to vector of front ends
                    _front_ends.push_back(ab);
                }

                /*!
                 * \brief Creates a new collection of alting barrier front ends associated with the base
                 *
                 * \param[in] ab Vector of alting barriers being created
                 */
                void expand(const std::vector<std::shared_ptr<alting_barrier_internal>> ab) noexcept
                {
                    // Add to enrolled and countdown
                    _enrolled += ab.size();
                    _countdown += ab.size();

                    // Add to currently enrolled front ends
                    _front_ends.insert(_front_ends.end(), ab.begin(), ab.end());
                }

                /*!
                 * \brief Removes an alting barrier front end from the list of associated front ends
                 *
                 * \param[in] ab The alting barrier to remove
                 */
                void contract(const std::shared_ptr<alting_barrier_internal> ab) noexcept(false);

                /*!
                 * \brief Removes a collection of alting barrier front ends from the base.
                 *
                 * \param[in] ab The vector of alting barriers to remove.
                 */
                void contract(const std::vector<std::shared_ptr<alting_barrier_internal>> &ab) noexcept(false);

                /*!
                 * \brief Enables the alting barrier base by signalling that one of the front ends is ready
                 *
                 * \return True if the alting barrier is ready, false otherwise
                 */
                bool enable() noexcept
                {
                    // Decrement countdown
                    --_countdown;

                    // Check if alting barrier is now ready
                    if (_countdown == 0)
                    {
                        // Signal all waiting front ends
                        _countdown = _enrolled;
                        alting_barrier_coordinate::start_disable(_enrolled);
                        for (auto &ab : _front_ends)
                            ab->schedule();

                        return true;
                    }
                    else
                        return false;
                }

                /*!
                 * \brief Disables the alting barrier by one of the front ends
                 *
                 * \return True if alting barrier is ready, false otherwise.
                 */
                bool disable() noexcept
                {
                    // Check if ready
                    if (_countdown == _enrolled)
                        return true;
                    else
                    {
                        ++_countdown;
                        return false;
                    }
                }

                /*!
                 * \brief Called when a front end resigns from the alting barrier
                 */
                void resign() noexcept;

                /*!
                 * \brief Enrolls a front end with alting barrier base after previously resigning
                 */
                void enroll() noexcept
                {
                    // Increment enrolled and countdown
                    ++_enrolled;
                    ++_countdown;
                }
            };

            std::shared_ptr<alting_barrier_base> _base = nullptr; //<! The alting barrier base which this alting barrier works upon

            alt _alt; //<! The alt that this alting barrier front end is being used with.

            bool _has_alt = false; //<! Flag to indicate if the alting barrier is currently being used.

            bool _owned = false; //<! Flag to indicate if the alting barrier is owned by a thread.

            std::thread::id _thread; //<! The thread that this alting barrier is owned by.

            bool _enrolled = true; //<! Flag indicating whether the alting barrier front end is owned.

            timer _poll_time; //<! Used to support the poll method.

            /*!
             * \brief Creates a new alting barrier front end.  Used internally by the framework.
             *
             * \param[in] base The base object of the alting barrier.
             */
            alting_barrier_internal(std::shared_ptr<alting_barrier_base> base) noexcept;

            /*!
             * \brief Creates a new alting barrier front end.
             *
             * \return A new alting barrier front end.
             */
            std::shared_ptr<alting_barrier_internal> expand() const noexcept;

            /*!
             * \brief Creates a new collection of alting barrier front ends.
             *
             * \return A vector of new front ends.
             */
            std::vector<std::shared_ptr<alting_barrier_internal>> expand(unsigned int n) const noexcept;

            /*!
             * \brief Removes an alting barrier front end from the alting barrier base.
             *
             * \param[in] ab The alting barrier to remove.
             */
            void contract(const std::shared_ptr<alting_barrier_internal> ab) const noexcept
            {
                // Just call contract on the base
                _base->contract(ab);
            }

            /*!
             * \brief Removes a collection of alting barrier front ends from the alting barrier.
             *
             * \param[in] ab The vector of alting barrier front ends to remove
             */
            void contract(const std::vector<std::shared_ptr<alting_barrier_internal>> &ab) const noexcept
            {
                // Just call contract on the base
                _base->contract(ab);
            }

            /*!
             * \brief Called when a process wishes to resign from the alting barrier
             */
            void resign() noexcept(false) override final
            {
                // Lock the alting barrier base to ensure only one process is in operation at a time.
                std::unique_lock<std::mutex> lock(_base->_mut);
                // Ensure that the alting barrier is actually enrolled
                if (!_enrolled)
                    throw std::runtime_error("alting barrier already resigned");

                // Resign from alting barrier base
                _enrolled = false;
                _base->resign();
            }

            /*!
             * \brief Called when a process wishes to enroll with the alting barrier.
             */
            void enroll() noexcept(false) override final
            {
                // Lock the barrier base
                std::unique_lock<std::mutex> lock(_base->_mut);
                // Ensure that alting barrier is not already enrolled.
                if (_enrolled)
                    throw std::runtime_error("alting barrier already enrolled");

                // Enroll with the alting barrier base
                _enrolled = true;
                _base->enroll();
            }

            /*!
             * \brief Used to indicate that an alting barrier front end has changed owner
             */
            void mark() noexcept(false)
            {
                // Lock the barrier base
                std::unique_lock<std::mutex> lock(_base->_mut);
                // Ensure the barrier is enrolled
                if (!_enrolled)
                    throw std::runtime_error("alting barrier is not enrolled");
                // Change thread owner
                _thread = std::this_thread::get_id();
            }

            /*!
             * \brief Called to indicate that the alting barrier front end should be reset to have no owner.
             */
            void reset() noexcept
            {
                // Lock the barrier base
                std::unique_lock<std::mutex> lock(_base->_mut);
                // Ensure the barrier is enrolled
                if (!_enrolled)
                {
                    _enrolled = true;
                    _base->enroll();
                }

                // Remove owner reference
                _owned = false;
            }

            /*!
             * \brief Performs a committed synchronization with the alting barrier.
             */
            void sync() noexcept override final
            {
                static alt _single_alt = alt{alting_barrier(shared_from_this())};
                // Perform a priority select on the single alt
                _single_alt.pri_select();
            }

            /*!
             * \brief Polls the alting barrier to check if it is ready
             *
             * \param[in] offer_time The amount of time to wait to see if the alting barrier becomes ready.
             *
             * \return True if the alting barrier becomes ready in the defined time, false otherwise.
             */
            bool poll(const std::chrono::steady_clock::duration &offer_time) noexcept
            {
                static alt _poll_alt = alt{alting_barrier(shared_from_this()), _poll_time};
                // Set the time to wait for the poll
                _poll_time += offer_time;
                // Return whether alting barrier became ready in this time.
                return (_poll_alt.pri_select() == 0);
            }

            /*!
             * \brief Schedules the front end with the alt
             */
            void schedule() noexcept
            {
                if (_has_alt)
                    guard_internal::schedule(_alt);
            }

            /*!
             * \brief Enables the alting barrier in an alt selection
             *
             * \param[in] a The alt being used to enable this alting barrier.
             *
             * \return True if the guard is ready, false otherwise.
             */
            bool enable(const alt &a) noexcept(false) override final
            {
                // Lock the base
                std::unique_lock<std::mutex> lock(_base->_mut);
                // Ensure alting barrier is in correct state
                if (!_owned)
                {
                    _owned = true;
                    _thread = std::this_thread::get_id();
                }
                else if (_thread != std::this_thread::get_id())
                    throw std::runtime_error("alting barrier not owned by this thread");
                if (_enrolled)
                    throw std::runtime_error("alting barrier not enrolled");

                // Check to make sure the alting barrier is only enabled once
                if (_has_alt)
                    return false;

                if (_base->enable())
                {
                    // Inform the alt that we are ready
                    set_barrier_trigger(a);
                    return true;
                }
                else
                {
                    _alt = a;
                    _has_alt = true;
                    return false;
                }
            }

            /*!
             * \brief Disables the alting barrier in an alt selection.
             *
             * \return True if the guard is ready, false otherwise.
             */
            bool disable() noexcept override final
            {
                // Lock the base
                std::unique_lock<std::mutex> lock(_base->_mut);
                // alting barrier may have occurred more than once
                if (!_has_alt)
                    return false;
                // Check if base has become ready
                if (_base->disable())
                {
                    set_barrier_trigger(_alt);
                    _has_alt = false;
                    return true;
                }
                else
                {
                    _has_alt = false;
                    return false;
                }
            }
        };

        std::shared_ptr<alting_barrier_internal> _internal = nullptr; //<! Pointer to the internal representation of the alting barrier.

        /*!
         * \brief Creates a new alting barrier.
         *
         * \param[in] internal Pointer to the internal representation of the alting barrier.
         */
        alting_barrier(std::shared_ptr<alting_barrier_internal> internal)
        : _internal(internal), barrier(internal), guard(internal)
        {
        }

    public:
        /*!
         * \brief Copy constructor.
         *
         * \param[in] other The alting barrier to copy.
         */
        alting_barrier(const alting_barrier &other) noexcept = default;

        /*!
         * \brief Move constructor.
         *
         * \param[in] rhs The alting barrier to copy.
         */
        alting_barrier(alting_barrier &&rhs) noexcept = default;

        /*!
         * \brief Copy assignment operator.
         *
         * \param[in] other The alting barrier to copy.
         *
         * \return The assigned to alting barrier.
         */
        alting_barrier& operator=(const alting_barrier &other) noexcept = default;

        /*!
         * \brief Move assignment operator.
         *
         * \param[in] rhs The alting barrier to copy.
         *
         * \return The assigned to alting barrier.
         */
        alting_barrier& operator=(alting_barrier &&rhs) noexcept = default;

        /*!
         * \brief Destroys the alting barrier.
         */
        ~alting_barrier() { }

        /*!
         * \brief Creates a new alting barrier with one prcess enrolled.
         *
         * \return A new alting barrier
         */
        static alting_barrier create() noexcept
        {
            // Create base
            auto base = std::shared_ptr<alting_barrier_internal::alting_barrier_base>(new alting_barrier_internal::alting_barrier_base());
            // Create front end
            auto internal = std::shared_ptr<alting_barrier_internal>(new alting_barrier_internal(base));
            // Expand base with new front end
            base->expand(internal);
            // Return new alting barrier with base
            return alting_barrier(internal);
        }

        /*!
         * \brief Creates a new alting barrier with an initial number of enrolled front ends.
         *
         * \param[in] n The number of front ends to create.
         *
         * \return A vector of new front ends.
         */
        static std::vector<alting_barrier> create(unsigned int n) noexcept
        {
            // Create base
            auto base = std::shared_ptr<alting_barrier_internal::alting_barrier_base>(new alting_barrier_internal::alting_barrier_base());
            // Create new vector of internals
            std::vector<std::shared_ptr<alting_barrier_internal>> ab(n);
            for (unsigned int i = 0; i < n; ++n)
                ab[i] = std::shared_ptr<alting_barrier_internal>(new alting_barrier_internal(base));
            // Exapand base with new front ends
            base->expand(ab);
            // Create vector of alting barriers
            std::vector<alting_barrier> to_return;
            for (auto &b : ab)
                to_return.push_back(alting_barrier(b));
            // Return new vector of alting barriers
            return to_return;
        }

        /*!
         * \brief Expands the number of processes enrolled with the alting barrier.
         *
         * \return A new front end.
         */
        alting_barrier expand() const noexcept(false)
        {
            // Lock the barrier base
            std::unique_lock<std::mutex> lock(_internal->_base->_mut);
            // Check that alting barrier is associated with a thread
            if (!_internal->_owned)
            {
                _internal->_thread = std::this_thread::get_id();
                _internal->_owned = true;
            }
            // If already associated then ensure used by correct thread
            else if (_internal->_thread != std::this_thread::get_id())
                throw std::runtime_error("alting barrier not owned by this thread");
            // Ensure alting barrier is enrolled
            if (!_internal->_enrolled)
                throw std::runtime_error("alting barrier not enrolled");
            // Expand the base
            return alting_barrier(_internal->expand());
        }

        /*!
         * \brief Expands the number of proceses enrolled with the alting barrier
         *
         * \param[in] n The number of new front ends to create.
         *
         * \return A vector of new front ends.
         */
        std::vector<alting_barrier> expand(unsigned int n) const noexcept(false)
        {
            // Lock the barrier base
            std::unique_lock<std::mutex> lock(_internal->_base->_mut);
            // Check that alting barrier is associated with a thread
            if (!_internal->_owned)
            {
                _internal->_thread = std::this_thread::get_id();
                _internal->_owned = true;
            }
            // If already associated then ensure used by correct thread
            else if (_internal->_thread != std::this_thread::get_id())
                throw std::runtime_error("alting barrier not owned by thread");
            // Ensure alting barrier is enrolled
            if (!_internal->_enrolled)
                throw std::runtime_error("alting barrier not enrolled");
            // Expand the base
            auto internals = _internal->expand(n);
            // Create vector of front ends to return
            std::vector<alting_barrier> to_return;
            for (auto &ab : internals)
                to_return.push_back(alting_barrier(ab));
            // Return vector
            return to_return;
        }

        /*!
         * \brief Contracts the alting barrier front end associated with the alting barrier.
         */
        void contract() const noexcept(false)
        {
            // Lock the base
            std::unique_lock<std::mutex> lock(_internal->_base->_mut);
            // Ensure that correct thread is using the alting barrier
            if (_internal->_thread != std::this_thread::get_id())
                throw std::runtime_error("alting barrier not owned by thread");
            // Ensure that alting barrier is enrolled
            if (_internal->_enrolled)
                throw std::runtime_error("alting barrier not enrolled");
            // Contract the base
            _internal->_base->contract(_internal);
        }

        /*!
         * \brief Contracts the alting barrier front ends associated with the alting barrier.
         *
         * \param[in] ab Vector of alting barriers to remove.
         */
        void contract(const std::vector<alting_barrier> &ab) const noexcept(false)
        {
            // Ensure valid parameter
            assert(ab.size() > 0);
            // Lock the base
            std::unique_lock<std::mutex> lock(_internal->_base->_mut);
            // Ensure that correct thread is using the alting barrier
            if (_internal->_thread != std::this_thread::get_id())
                throw std::runtime_error("alting barrier not owned by thread");
            // Ensure that alting barrier is enrolled
            if (_internal->_enrolled)
                throw std::runtime_error("alting barrier not enrolled");
            // Extract the internal objects
            std::vector<std::shared_ptr<alting_barrier_internal>> internals;
            for (auto &b : ab)
                internals.push_back(b._internal);
            // Contract the base
            _internal->_base->contract(internals);
        }

        /*!
         * \brief Used to indicate that an alting barrier front end has changed owner.
         */
        void mark() const noexcept { _internal->mark(); }

        /*!
         * \brief Polls the alting barrier to check if ready
         *
         * \param[in] offer_time The amount of time to poll the alting barrier for
         *
         * \return True if the barrier is ready, false otherwise.
         */
        bool poll(const std::chrono::steady_clock::duration &offer_time) const noexcept { return _internal->poll(offer_time); }
    };

    void alting_barrier::alting_barrier_internal::alting_barrier_base::contract(const std::shared_ptr<alting_barrier_internal> ab) noexcept(false)
    {
        // Find front end in vector
        auto found = std::find(_front_ends.begin(), _front_ends.end(), ab);
        if (found == _front_ends.end())
            // Not found
            throw std::invalid_argument("Could not find front end in alting barrier");

        // Set alting barrier base pointed to by alting barrier
        ab->_base = nullptr;

        // Decrement enrolled and countdown if necessary
        if (ab->_enrolled)
        {
            --_enrolled;
            --_countdown;
        }

        // Check if the removal has caused the alting barrier to become ready.
        if (_countdown == 0)
        {
            _countdown = _enrolled;

            // If there are processes enrolled then signal them
            if (_enrolled > 0)
            {
                alting_barrier_coordinate::start_enable();
                alting_barrier_coordinate::start_disable(_enrolled);

                for (auto &b : _front_ends)
                    b->schedule();
            }
        }
    }

    void alting_barrier::alting_barrier_internal::alting_barrier_base::contract(const std::vector<std::shared_ptr<alting_barrier_internal>> &ab) noexcept(false)
    {
        unsigned int discarded = 0, found = 0;
        for (auto &b : ab)
        {
            // Find front end
            auto found = std::find(_front_ends.begin(), _front_ends.end(), b);
            if (found != _front_ends.end())
            {
                // Front end found.  Erase and increment found.
                b->_base = nullptr;
                _front_ends.erase(found);
                ++found;
                // Increment discarded if required
                if (b->_enrolled)
                    ++discarded;
            }
        }

        // Check that enough front ends were found.
        if (found != ab.size())
            throw std::invalid_argument("Not all front ends where associated with this barrier");

        // Reduce enrolled and countdown by discarded
        _enrolled -= discarded;
        _countdown -= discarded;

        // Check if countdown has hit 0
        if (_countdown == 0)
        {
            _countdown = _enrolled;
            if (_enrolled > 0)
            {
                alting_barrier_coordinate::start_enable();
                alting_barrier_coordinate::start_disable(_enrolled);
                for (auto &b : _front_ends)
                    b->schedule();
            }
        }
    }

    void alting_barrier::alting_barrier_internal::alting_barrier_base::resign() noexcept
    {
        // Decrement enrolled and countdown
        --_enrolled;
        --_countdown;

        // Check if alting barrier has now become ready
        if (_countdown)
        {
            _countdown = _enrolled;

            // If enrolled > 0 then signal waiting front ends
            if (_enrolled > 0)
            {
                alting_barrier_coordinate::start_enable();
                alting_barrier_coordinate::start_disable(_enrolled);
                for (auto &ab : _front_ends)
                    ab->schedule();
            }
        }
    }

    alting_barrier::alting_barrier_internal::alting_barrier_internal(std::shared_ptr<alting_barrier_base> base) noexcept
    : _base(base)
    {
    }

    std::shared_ptr<alting_barrier::alting_barrier_internal> alting_barrier::alting_barrier_internal::expand() const noexcept
    {
        // Create new front end
        std::shared_ptr<alting_barrier_internal> ab(new alting_barrier_internal(_base));
        // Call expand on the base
        _base->expand(ab);
        // Return new front end
        return ab;
    }

    std::vector<std::shared_ptr<alting_barrier::alting_barrier_internal>> alting_barrier::alting_barrier_internal::expand(unsigned int n) const noexcept
    {
        // Create new vector of alting barriers
        std::vector<std::shared_ptr<alting_barrier::alting_barrier_internal>> ab;
        for (unsigned int i = 0; i < n; ++i)
            ab.push_back(std::shared_ptr<alting_barrier::alting_barrier_internal>(new alting_barrier::alting_barrier_internal(_base)));
        // Call expand on the base
        _base->expand(ab);
        // Return new front ends
        return ab;
    }
}

#endif //CPP_CSP_ALTING_BARRIER_H
