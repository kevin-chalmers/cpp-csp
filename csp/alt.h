//
// Created by kevin on 07/04/16.
//

#ifndef CPP_CSP_ALT_H
#define CPP_CSP_ALT_H

#include <memory>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <initializer_list>
#include <chrono>
#include <type_traits>
#include <cassert>
#include "guard.h"

namespace csp
{
    // Forward declaration of alting barrier
    class alting_barrier;

    /*! \class multiway_sync
     * \brief An interface class that defines an object as being a multiway sync.
     * This allows selection coordination via a number of processes.
     *
     * \author Kevin Chalmers
     * \date 7/4/2016
     */
    class multiway_sync
    {
    public:
        /*!
         * \brief Virtual destructor.  This is an interface class used for
         * type checking.
         */
        virtual ~multiway_sync() { }
    };

    /*! \class alt
     * \brief Used within processes to enable selection between a set of events.
     *
     * The alt class allows the selection of one ready event from a set of events
     * that the process is willing to engage in.  This allows a process to determine
     * its behaviour based on external factors (an external choice in CSP terms).
     * Using this class allows library users to write code in a similar manner to
     * Erlang pattern matching, but with far more flexibility in the range of events
     * that the process is willing to negotiate on.
     *
     * \author Kevin Chalmers
     * \date 8/4/2016
     */
    class alt
    {
        friend class guard;
    private:
        /*!
         * \class alt_internal
         * \brief The internal representation of an alt.  Used within the framework
         * to perform the actual selection operations.
         *
         * \author Kevin Chalmers
         * \date 8/4/2016
         */
        class alt_internal : public std::enable_shared_from_this<alt_internal>
        {
        private:
            std::mutex _mut; //!< Mutex used to control access to the alt

            std::condition_variable _cond; //!< Condition variable used to coordinate the mutex

            /*! \enum STATE
             * \brief Defines the possible states that the alt can be in
             */
            enum class STATE
            {
                ENABLING    = 0,    //!< alt is currently enabling guards
                WAITING     = 1,    //!< alt is currently waiting for an event
                READY       = 2,    //!< alt is ready
                INACTIVE    = 3,    //!< alt is inactive
            };

            STATE _state = STATE::INACTIVE; //<! Current state of the alt

            std::vector<guard> _guards; //<! The guards currently associated with the alt

            int _next = 0; //<! The index of the guard that has the highest priority during the next selection

            static constexpr int NONE_SELECTED = -1; //<! The value used if no guard is selected

            int _selected = NONE_SELECTED; //<! The index of the currently selected guard

            bool _barrier_present = false; //<! Flag to indicate if an alting barrier is present

            bool _barrier_trigger = false; //<! Flag to indicate successful enable / disable of alting barrier

            int _barrier_selected = NONE_SELECTED; //<! Index of the selected alting barrier

            int _enable_index = -1; //<! Index variable used during enable / disable sequences

            bool _timeout = false; //<! Flag to indicate if one of the guards is a timer

            std::chrono::steady_clock::time_point _time; //<! Hold the earliest timer value

            int _timer_index = -1; //<! Index of the timer with the earliest timeout

            /*!
             * \brief Internal operation to perform the selection of guards
             *
             * \return The index of the selected guard in the vector _guards
             */
            int do_select() noexcept;

            /*!
             * \brief Internal operation to perform the selection of guards with pre-conditions
             *
             * \param[in] pre_conditions The list of pre-conditions used to determine if a guard
             * should be involved in the selection operation.
             *
             * \return The index of the selected guard in the vector _guards
             */
            int do_select(const std::vector<bool> &pre_conditions) noexcept;

            /*!
             * \brief Internal operation used to enable guards for selection
             */
            void enable_guards() noexcept;

            /*!
             * \brief Internal operation used to enable guards for selection with a list of pre-conditions
             *
             * \param[in] pre_conditions The list of pre-conditions on the guards
             */
            void enable_guards(const std::vector<bool> &pre_conditions) noexcept;

            /*!
             * \brief Internal operation used to disable guards at the end of selection.
             */
            void disable_guards() noexcept(false);

            /*!
             * \brief Internal operation used to diable guards at the end of selection, with
             * specified pre-conditions.
             *
             * \param[in] pre_conditions The list of pre-conditions on the guards
             */
            void disable_guards(const std::vector<bool> &pre_conditions) noexcept(false);

        public:
            /*!
             * \brief Creates a new alt_internal with the given vector of guards
             *
             * \param[in] guards The list of guards that the alt will select on.
             */
            alt_internal(const std::vector<guard> &guards) noexcept
            : _guards(guards)
            {
                // Determine if we have a multiway sync
                for (auto &g : _guards)
                {
                    if (dynamic_cast<multiway_sync*>(g._internal.get()))
                    {
                        _barrier_present = true;
                        return;
                    }
                }
            }

            /*!
             * \brief Creates a new alt_iternal with the given vector of guards, using move semantics
             *
             * \param[in] guards The list of guards that thw alt will select on.
             */
            alt_internal(std::vector<guard> &&guards) noexcept
            : _guards(guards)
            {
                // Determine if we have a multiway sync
                for (auto &g : _guards)
                {
                    if (dynamic_cast<multiway_sync*>(g._internal.get()))
                    {
                        _barrier_present = true;
                        return;
                    }
                }
            }

            /*!
             * \brief Performs a select operation on the guards.
             *
             * \return The index of the selected guard in the _guards vector.
             */
            int select() noexcept;

            /*!
             * \brief Performs a select operation on the guards with a list of given pre-conditions.
             *
             * \param[in] pre_conditions The list of pre-conditions for the guards.
             *
             * \return The index of the selected guard in the _guards vector.
             */
            int select(const std::vector<bool> &pre_conditions) noexcept;

            /*!
             * \brief Performs a selection operation on the guards using a priority.
             *
             * \return The index of the selected guard in the _guards vector.
             */
            int pri_select() noexcept;

            /*!
             * \brief Performs a selection operation on the guards using a priorty and with a list of pre-conditions.
             *
             * \param[in] pre_conditions The list of pre-conditions for the guards.
             *
             * \return The index of the selected guard in the _guards vector.
             */
            int pri_select(const std::vector<bool> &pre_conditions) noexcept;

            /*!
             * \brief Performs a fair selection operation on the guards.
             *
             * \return The index of the selected guard in the _guards vector.
             */
            int fair_select() noexcept;

            /*!
             * \brief Performs a fair selection operation on the guards using a list of pre-codntions.
             *
             * \param[in] pre_conditions The list of pre-conditions for the guards.
             *
             * \return The index of the selected guard in the _guards vector.
             */
            int fair_select(const std::vector<bool> &pre_conditions) noexcept;

            /*!
             * \brief Sets the _barrier_trigger_flag.
             */
            void set_barrier_trigger() noexcept { _barrier_trigger = 0; }

            /*!
             * \brief Called by the timer when it is enabled as a guard
             *
             * \param[in] time The time to schedule for the alt to become activated.
             */
            void set_timeout(const std::chrono::steady_clock::time_point &time) noexcept
            {
                // Check if a timer is already registered.
                if (_timeout)
                {
                    // If timer already present check whether the new one will fire first
                    if (time < _time)
                    {
                        // Set the new time
                        _time = time;
                        _timer_index = _enable_index;
                    }
                }
                else
                {
                    // Otherwise activate this time
                    _timeout = true;
                    _time = time;
                    _timer_index = _enable_index;
                }
            }

            /*!
             * \brief Called by a guard to indicate that it has become ready.
             */
            void schedule() noexcept;

            /*!
             * \brief Destroys the alt_internal.
             */
            ~alt_internal() { }
        };

        std::shared_ptr<alt_internal> _internal = nullptr; //<! Pointer to the internal representaiton of the alt.

        /*!
         * \brief Sets the barrier trigger.  This is called by a barrier when it has become ready.
         */
        void set_barrier_trigger() const noexcept { _internal->set_barrier_trigger(); }

        /*!
         * \brief Sets the timer when it is enabled by a guard.
         *
         * \param[in] time The time to activate the alt at.
         */
        void set_timeout(const std::chrono::steady_clock::time_point &time) const noexcept { _internal->set_timeout(time); }

        /*!
         * \brief Called by a guard to indicate that it has become ready.  Used by guard objects
         * external to the alt.
         */

        void schedule() const noexcept { _internal->schedule(); }

        /*!
         * \brief Private constructor used by alt_internal
         *
         * \param[in] internal Shared pointer to the alt_internal
         */
        alt(std::shared_ptr<alt_internal> internal)
        : _internal(internal)
        {
        }

    public:

        /*!
         * \brief Creates a new alt.
         */
        alt() noexcept { }

        /*!
         * \brief Creates a new alt with the given list of guards
         *
         * \param[in] guards The guards that the alt will operate on.
         */
        alt(const std::initializer_list<guard> &guards) noexcept
        {
            _internal = std::shared_ptr<alt_internal>(new alt_internal(guards));
        }

        /*!
         * \brief Creates a new alt with the given list of guards.
         *
         * \param[in] guards The guards that the alt will operate on.
         */
        alt(const std::vector<guard> &guards) noexcept
        {
            _internal = std::shared_ptr<alt_internal>(new alt_internal(guards));
        }

        /*!
         * \brief Creates a new alt, generating the guard list from the given iterator.
         *
         * \tparam _Iter The iterator type used for list generation.  This must work across guards.
         *
         * \param[in] begin The start of the iterator.
         * \param[in] end The end of the iterator.
         */
        template<typename _Iter>
        alt(const _Iter &begin, const _Iter &end) noexcept
        {
            _internal = std::shared_ptr<alt_internal>(new alt_internal(std::vector<guard>(begin, end)));
        }

        /*!
         * \brief Copy constructor.
         *
         * \param[in] other The alt to be copied.
         */
        alt(const alt &other) noexcept = default;

        /*!
         * \brief Move constructor.
         *
         * \param[in] rhs The alt to be copied.
         */
        alt(alt &&rhs) noexcept = default;

        /*!
         * \brief Destroys the alt.
         */
        ~alt() noexcept { }

        /*!
         * \brief Copy assignment operator.
         *
         * \param[in] other The alt to be copied.
         */
        alt& operator=(const alt &other) noexcept = default;

        /*!
         * \brief Move assignment operator.
         *
         * \param[in] rhs The alt to be copied.
         */
        alt& operator=(alt &&rhs) noexcept = default;

        /*!
         * \brief Performs a select operation on the guards.  Will use fair selection semantics.
         *
         * \return The index of the selected guard.
         */
        int select() const noexcept { return _internal->select(); }

        /*!
         * \brief Performs a select operation on the guards using the given pre-conditions.  Will use fair selection semantics.
         *
         * \param[in] pre_conditions List of pre-conditions to use with the guards.
         *
         * \return The index of the selected guard.
         */
        int select(const std::vector<bool> &pre_conditions) const noexcept { return _internal->select(pre_conditions); }

        /*!
         * \brief Performs a selection operation on the guards.  Uses a priority selection.
         *
         * Priority selection means that the first guard in the list will be tested first, followed by the second, etc.
         * This is unlike the normal select (and fair select) where the next guard after the previously selected one
         * will be enabled first.
         *
         * \return The index of the selected guard.
         */
        int pri_select() const noexcept { return _internal->pri_select(); }

        /*!
         * \brief Performs a selection operation on the guards using the given pre-conditions.  Uses a priority selection.
         *
         * Priority selection means that the first guard in the list will be tested first, followed by the second, etc.
         * This is unlike the normal select (and fair select) where the next guard after the previously selected one
         * will be enabled first.
         *
         * \param[in] pre_conditions List of pre-conditions to use with the guards.
         *
         * \return The index of the selected guard.
         */
        int pri_select(const std::vector<bool> &pre_conditions) const noexcept { return _internal->pri_select(pre_conditions); }

        /*!
         * \brief Performs a fair selection operation on the guards.
         *
         * \return The index of the selected guard.
         */
        int fair_select() const noexcept { return _internal->fair_select(); }

        /*!
         * \brief Performs a fair selection operation on the guards using the given pre-conditions.
         *
         * \param[in] pre_conditions List of pre-conditions to use with the guards.
         *
         * \return The index of the selected guard.
         */
        int fair_select(const std::vector<bool> &pre_conditions) const noexcept { return _internal->fair_select(pre_conditions); }

        /*!
         * \brief Calls select on the alt.
         *
         * \return The index of the selected guard.
         */
        int operator()() const noexcept { return select(); }

        /*!
         * \brief Calls select on the alt with the list of pre-conditions
         *
         * \param[in] pre_conditions List of pre-conditions to use with the guard.
         *
         * \return The index of the selected guard.
         */
        int operator()(const std::vector<bool> &pre_conditions) const noexcept { return select(pre_conditions); }
    };

    /*! \class alting_barrier_coordinate
     * \brief A static helper class used to coordinate alting barriers
     * Used internally by the framework.
     *
     * \author Kevin Chalmers
     *
     * \date 8/4/2016
     */
    class alting_barrier_coordinate
    {
        friend class alt;
        friend class alting_barrier;
    private:
        static int _active; //!< The number of processes active in the alt.

        static std::unique_ptr<std::mutex> _active_lock; //!< Mutex for coordinating enable / disable sequences.

        static std::unique_ptr<std::condition_variable> _cond; //!< Condition varianle for coordinate enable / disable sequences.

        /*!
         * \brief Starts an enable sequence on an alting_barrier.
         */
        static void start_enable() noexcept(false)
        {
            // Ensure only one enable sequence is in effect.
            std::unique_lock<std::mutex> lock(*_active_lock);

            // Ensure we haven't woken up when we shouldn't
            while (_active > 0)
                _cond->wait(lock);

            // Sanity check
            if (_active != 0)
                throw std::runtime_error("alting_barrier enable sequence starting with active count not equal to zero: " + std::to_string(_active));

            // Set active to one to ensure we are the only active enable
            _active = 1;
        }

        /*!
         * \brief Ends an enable sequence on an alting_barrier.
         */
        static void finish_enable() noexcept(false)
        {
            // Ensure only one enable sequence is in effect.
            std::unique_lock<std::mutex> lock(*_active_lock);

            // Sanity check.  Ensure that we are the only enable in effect.
            if (_active != 1)
                throw std::runtime_error("alting barrier enable sequence finished with active count not equal to zero: " + std::to_string(_active));

            // Set active to 0 to indicate enable has completed.
            _active = 0;

            // Signal the next waiting process (if any) so that they can start an enable.
            _cond->notify_one();
        }

        /*!
         * \brief Starts a disable sequence on an alting barrier.
         *
         * \param[in] n Number of front ends to disable
         */
        static void start_disable(int n) noexcept(false)
        {
            // Assert n is > 0
            assert(n > 0);

            // Ensure that only one process is in operation
            std::unique_lock<std::mutex> lock(*_active_lock);

            // Sanity check.  Ensure that no other process is disabling
            if (_active != 1)
                throw std::runtime_error("Completed alting barrier found in alt sequence with active count not equal to one: " + std::to_string(_active));

            // Set active value to number of front-ends to disable
            _active = n;
        }

        /*!
         * \brief Completes a disable sequence.
         */
        static void finish_disable() noexcept(false)
        {
            // Ensure that we are the only process in operation
            std::unique_lock<std::mutex> lock(*_active_lock);

            // Ensure that we aren't disabling too many front ends.
            if (_active < 1)
                throw std::runtime_error("alting barrier disable sequence finished with active count less than one: " + std::to_string(_active));

            // Decrement active count
            --_active;

            // If active count has hit 0 then inform the next waiting process
            if (_active == 0)
                _cond->notify_one();
        }
    };

    // Initialise static values
    int alting_barrier_coordinate::_active = 0;
    std::unique_ptr<std::mutex> alting_barrier_coordinate::_active_lock = std::unique_ptr<std::mutex>(new std::mutex());
    std::unique_ptr<std::condition_variable> alting_barrier_coordinate::_cond = std::unique_ptr<std::condition_variable>(new std::condition_variable());

    int alt::alt_internal::do_select() noexcept
    {
        // Set state to ENABLING
        _state = STATE::ENABLING;

        // Enable all guards
        enable_guards();

        {
            // Lock the alt
            std::unique_lock<std::mutex> lock(_mut);

            // Perform the alt operation
            // Check if we are still enabling
            if (_state == STATE::ENABLING)
            {
                // Set state to waiting
                _state = STATE::WAITING;
                // Check if we have a timeout
                if (_timeout)
                {
                    // Have a timeout.  Wait until time or guard becomes ready.
                    _cond.wait_until(lock, _time);
                }
                else
                {
                    // No timeout.  Wait until ready guard.
                    _cond.wait(lock);
                }

                // Set state to READY
                _state = STATE::READY;
            }
            // End of block - _mut will be released
        }

        // Disable the guards
        disable_guards();

        // Set state to inactive
        _state = STATE::INACTIVE;
        _timeout = false;

        // Return the currently selected guard.
        return _selected;
    }

    int alt::alt_internal::do_select(const std::vector<bool> &pre_conditions) noexcept
    {
        // Set state to ENABLING
        _state = STATE::ENABLING;

        // Enable all guards
        enable_guards(pre_conditions);

        // Open block.  Allows use of lock guard
        {
            // Lock the alt
            std::unique_lock<std::mutex> lock(_mut);

            // Perform the alt operation
            // Check if we are still enabling
            if (_state == STATE::ENABLING)
            {
                // Set state to waiting
                _state = STATE::WAITING;
                // Check if we have a timeout
                if (_timeout)
                {
                    // Have a timeout.  Wait until time or guard becomes ready.
                    _cond.wait_until(lock, _time);
                }
                else
                {
                    // No timeout.  Wait until ready guard.
                    _cond.wait(lock);
                }

                // Set state to READY
                _state = STATE::READY;
            }
            // End of block - _mut will be released
        }

        // Disable the guards
        disable_guards(pre_conditions);

        // Set state to inactive
        _state = STATE::INACTIVE;
        _timeout = false;

        // Return the currently selected guard.
        return _selected;
    }

    void alt::alt_internal::enable_guards() noexcept
    {
        // If alting barrier exists then coordinate
        if (_barrier_present)
            alting_barrier_coordinate::start_enable();

        // Set the currently selected barrier
        _barrier_selected = NONE_SELECTED;

        // Iterate through the guards and enable each in turn
        for (_enable_index = _next; _enable_index < int(_guards.size()); ++_enable_index)
        {
            // Enable guard and check if ready
            alt temp(shared_from_this());
            if (_guards[_enable_index].enable(temp))
            {
                // If guard is ready set the selected index
                _selected = _enable_index;
                _state = STATE::READY;

                // Check if a barrier has triggered
                if (_barrier_trigger)
                {
                    _barrier_selected = _selected;
                    _barrier_trigger = false;
                }
                // Else if barrier is present finish enable
                else if (_barrier_present)
                    alting_barrier_coordinate::finish_enable();
                return;
            }
        }
        for (_enable_index = 0; _enable_index < _next; ++_enable_index)
        {
            // Enable guard and check if ready
            alt temp(shared_from_this());
            if (_guards[_enable_index].enable(temp))
            {
                // If guard is ready set the selected index
                _selected = _enable_index;
                _state = STATE::READY;

                // Check if a barrier has triggered
                if (_barrier_trigger)
                {
                    _barrier_selected = _selected;
                    _barrier_trigger = false;
                }
                    // Else if barrier is present finish enable
                else if (_barrier_present)
                    alting_barrier_coordinate::finish_enable();
                return;
            }
        }

        // No guard was ready.  Set selected to NONE_SELECTED
        _selected = NONE_SELECTED;

        // If alting barrier present finish enable
        if (_barrier_present)
            alting_barrier_coordinate::finish_enable();
    }

    void alt::alt_internal::enable_guards(const std::vector<bool> &pre_conditions) noexcept
    {
        // If alting barrier exists then coordinate
        if (_barrier_present)
            alting_barrier_coordinate::start_enable();

        // Set the currently selected barrier
        _barrier_selected = NONE_SELECTED;

        // Iterate through the guards and enable each in turn
        for (_enable_index = _next; _enable_index < int(_guards.size()); ++_enable_index)
        {
            // Enable guard and check if ready
            alt temp(shared_from_this());
            if (pre_conditions[_enable_index] && _guards[_enable_index].enable(temp))
            {
                // If guard is ready set the selected index
                _selected = _enable_index;
                _state = STATE::READY;

                // Check if a barrier has triggered
                if (_barrier_trigger)
                {
                    _barrier_selected = _selected;
                    _barrier_trigger = false;
                }
                    // Else if barrier is present finish enable
                else if (_barrier_present)
                    alting_barrier_coordinate::finish_enable();
                return;
            }
        }
        for (_enable_index = 0; _enable_index < _next; ++_enable_index)
        {
            // Enable guard and check if ready
            alt temp(shared_from_this());
            if (pre_conditions[_enable_index] && _guards[_enable_index].enable(temp))
            {
                // If guard is ready set the selected index
                _selected = _enable_index;
                _state = STATE::READY;

                // Check if a barrier has triggered
                if (_barrier_trigger)
                {
                    _barrier_selected = _selected;
                    _barrier_trigger = false;
                }
                    // Else if barrier is present finish enable
                else if (_barrier_present)
                    alting_barrier_coordinate::finish_enable();
                return;
            }
        }

        // No guard was ready.  Set selected to NONE_SELECTED
        _selected = NONE_SELECTED;

        // If alting barrier present finish enable
        if (_barrier_present)
            alting_barrier_coordinate::finish_enable();
    }

    void alt::alt_internal::disable_guards() noexcept(false)
    {
        // First check if there is anything to disable
        if (_selected != _next)
        {
            // Set the start index to start disabling
            int start_index = (_selected == NONE_SELECTED ? _next - 1 : _selected - 1);
            // Iterate through the guards and disable them
            if (_selected < _next)
            {
                // Disable guards
                for (auto i = start_index; i >= 0; --i)
                {
                    // Disable guard and check if ready
                    if (_guards[i].disable())
                    {
                        // Set selected guard
                        _selected = i;
                        // Check if it is a barrier that has triggered
                        if (_barrier_trigger)
                        {
                            // Sanity check
                            if (_barrier_selected != NONE_SELECTED)
                                throw std::runtime_error("Second alting barrier completed in alt sequence: "
                                                         + std::to_string(_barrier_selected) + std::string(" and ") +
                                                         std::to_string(i));
                            // Set barrier selected
                            _barrier_selected = _selected;
                            _barrier_trigger = false;
                        }
                    }
                }
                start_index = int(_guards.size()) - 1;
            }
            for (auto i = start_index; i >= _next; --i)
            {
                // Disable guards
                for (auto i = start_index; i >= 0; --i)
                {
                    // Disable guard and check if ready
                    if (_guards[i].disable())
                    {
                        // Set selected guard
                        _selected = i;
                        // Check if it is a barrier that has triggered
                        if (_barrier_trigger)
                        {
                            // Sanity check
                            if (_barrier_selected != NONE_SELECTED)
                                throw std::runtime_error("Second alting barrier completed in alt sequence: "
                                                         + std::to_string(_barrier_selected) + std::string(" and ") +
                                                         std::to_string(i));
                            // Set barrier selected
                            _barrier_selected = _selected;
                            _barrier_trigger = false;
                        }
                    }
                }
                start_index = int(_guards.size()) - 1;
            }
            // Check if guard is selected.  If not, this implies that the timer fired
            if (_selected == NONE_SELECTED)
                _selected = _timer_index;
        }

        // Check for barrier sync to ensure that this is selected
        if (_barrier_selected != NONE_SELECTED)
        {
            _selected = _barrier_selected;
            alting_barrier_coordinate::finish_enable();
        }
    }

    void alt::alt_internal::disable_guards(const std::vector<bool> &pre_conditions) noexcept(false)
    {
        // First check if there is anything to disable
        if (_selected != _next)
        {
            // Set the start index to start disabling
            int start_index = (_selected == NONE_SELECTED ? _next - 1 : _selected - 1);
            // Iterate through the guards and disable them
            if (_selected < _next)
            {
                // Disable guards
                for (auto i = start_index; i >= 0; --i)
                {
                    // Disable guard and check if ready
                    if (pre_conditions[i] && _guards[i].disable())
                    {
                        // Set selected guard
                        _selected = i;
                        // Check if it is a barrier that has triggered
                        if (_barrier_trigger)
                        {
                            // Sanity check
                            if (_barrier_selected != NONE_SELECTED)
                                throw std::runtime_error("Second alting barrier completed in alt sequence: "
                                                         + std::to_string(_barrier_selected) + std::string(" and ") +
                                                         std::to_string(i));
                            // Set barrier selected
                            _barrier_selected = _selected;
                            _barrier_trigger = false;
                        }
                    }
                }
                start_index = int(_guards.size()) - 1;
            }
            for (auto i = start_index; i >= _next; --i)
            {
                // Disable guards
                for (auto i = start_index; i >= 0; --i)
                {
                    // Disable guard and check if ready
                    if (pre_conditions[i] && _guards[i].disable())
                    {
                        // Set selected guard
                        _selected = i;
                        // Check if it is a barrier that has triggered
                        if (_barrier_trigger)
                        {
                            // Sanity check
                            if (_barrier_selected != NONE_SELECTED)
                                throw std::runtime_error("Second alting barrier completed in alt sequence: "
                                                         + std::to_string(_barrier_selected) + std::string(" and ") +
                                                         std::to_string(i));
                            // Set barrier selected
                            _barrier_selected = _selected;
                            _barrier_trigger = false;
                        }
                    }
                }
                start_index = int(_guards.size()) - 1;
            }
            // Check if guard is selected.  If not, this implies that the timer fired
            if (_selected == NONE_SELECTED)
                _selected = _timer_index;
        }

        // Check for barrier sync to ensure that this is selected
        if (_barrier_selected != NONE_SELECTED)
        {
            _selected = _barrier_selected;
            alting_barrier_coordinate::finish_enable();
        }
    }

    int alt::alt_internal::select() noexcept
    {
        return fair_select();
    }

    int alt::alt_internal::select(const std::vector<bool> &pre_conditions) noexcept
    {
        assert(pre_conditions.size() == _guards.size());
        return fair_select(pre_conditions);
    }

    int alt::alt_internal::pri_select() noexcept
    {
        // Ensure that first guard gains priority
        _next = 0;
        // Perform select operation
        return do_select();
    }

    int alt::alt_internal::pri_select(const std::vector<bool> &pre_conditions) noexcept
    {
        assert(pre_conditions.size() == _guards.size());
        // Ensure that tthat first guard gains priority
        _next = 0;
        // Perform select operation
        return do_select(pre_conditions);
    }

    int alt::alt_internal::fair_select() noexcept
    {
        // Perform select operation
        int to_return = do_select();
        // Set next priority guard
        _next = (_next == _guards.size()) ? 0 : to_return + 1;
        return to_return;
    }

    int alt::alt_internal::fair_select(const std::vector<bool> &pre_conditions) noexcept
    {
        assert(pre_conditions.size() == _guards.size());
        // Perform select operation
        int to_return = do_select(pre_conditions);
        // Set next priority guard
        _next = (_next == _guards.size()) ? 0 : to_return;
        return to_return;
    }

    void alt::alt_internal::schedule() noexcept
    {
        // Lock the mutex
        std::unique_lock<std::mutex> lock(_mut);

        // Depending on state of the alt set state to ready
        switch (_state)
        {
            case STATE::ENABLING:
                _state = STATE::READY;
                break;
            case STATE::WAITING:
                _state = STATE::READY;
                _cond.notify_one();
                break;
            default:
                break;
        }
    }

    void guard::guard_internal::schedule(const alt &a) const noexcept { a._internal->schedule(); }

    void guard::guard_internal::set_timeout(const alt &a, const std::chrono::steady_clock::time_point &time) const noexcept { a._internal->set_timeout(time); }

    void guard::guard_internal::set_barrier_trigger(const alt &a) const noexcept { a._internal->set_barrier_trigger(); }

    class choice
    {
    private:
        std::vector<guard> _guards;
        std::vector<bool> _precond;
        std::vector<std::function<void()>> _funs;
    public:
        choice(std::initializer_list<std::tuple<guard, std::function<void()>>> &&select_list) noexcept
        {
            for (auto &entry : select_list)
            {
                _guards.push_back(std::get<0>(entry));
                _funs.push_back(std::get<1>(entry));
            }
        }

        choice(std::initializer_list<std::tuple<guard, bool, std::function<void()>>> &&select_list) noexcept
        {
            for (auto &entry : select_list)
            {
                _guards.push_back(std::get<0>(entry));
                _precond.push_back(std::get<1>(entry));
                _funs.push_back(std::get<2>(entry));
            }
        }

        void operator()() noexcept
        {
            if (_precond.size() == 0)
            {
                auto idx = alt(_guards)();
                _funs[idx]();
            }
            else
            {
                auto idx = alt(_guards)(_precond);
                _funs[idx]();
            }
        }
    };
}

#endif //CPP_CSP_ALT_H
