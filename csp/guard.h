//
// Created by kevin on 07/04/16.
//

#ifndef CPP_CSP_GUARD_H
#define CPP_CSP_GUARD_H

#include <chrono>
#include <memory>

namespace csp
{
    // Forward declarations
    class alt;

    /*! \class guard
     * \brief Interface class used to denote that an object can act
     * as a guard in a selection operation
     *
     * A guard object is one that can be utilised in a selection operation
     * (using an alt).  This allows a process to chose from a set of
     * events that are associated with the process.
     *
     * \author Kevin Chalmers
     * \date 7/4/2016
     */
    class guard
    {
        friend class alt;
    protected:
        /*! \class guard_internal
         * \brief Internal representation of a guarded object.  Used by
         * internal objects that wish to act as guards.
         *
         * \author Kevin Chalmers
         * \date 7/4/2016
         */
        class guard_internal
        {
        public:
            /*!
             * \brief Enables the guarded object with an alt.  Used
             * internally during selection operations.
             *
             * \param[in,out] a The alt being used in the selection process
             * \return True if the guard is currently ready to be selected, false otherwise
             */
            virtual bool enable(const alt &a) noexcept(false) = 0;

            /*!
             * \brief Disables the guarded object from an alt.  Used
             * internally during the end of selection operations.
             *
             * \return True if the guard is currently ready to be selected, false otherwise
             */
            virtual bool disable() noexcept(false) = 0;

            /*!
             * \brief Allows a subclass to schedule with the alt.  Helper
             * method to overcome some of the class protection levels.
             *
             * \param[in,out] a The alt schedule is being called on.
             */
            void schedule(const alt &a) const noexcept;

            /*!
             * \brief Allows a subclass to set the timeout with the alt.
             * Helper method to overcome some of the class protection levels.
             *
             * \param[in,out] a The alt that the timeout is to be set on.
             * \param[in] time The time that is to be set.
             */
            void set_timeout(const alt &a, const std::chrono::steady_clock::time_point &time) const noexcept;

            /*!
             * \brief Allows a subclass to set the barrier trigger flag with the alt.
             * Helper method to overcome some of the class protection levels.
             *
             * \param[in,out] a The alt that the barrier flag needs to be set on.
             */
            void set_barrier_trigger(const alt &a) const noexcept;

            /*!
             * \brief Destroys the guarded object
             */
            virtual ~guard_internal() { }
        };

        std::shared_ptr<guard_internal> _internal = nullptr; //!< Pointer to the object being guarded

        /*!
         * \brief Protected constructor.  Cannot be created explicitly, but by child classes.
         */
        guard(std::shared_ptr<guard_internal> internal) noexcept
        : _internal(internal)
        {
        }

    public:
        /*!
         * \brief Copy constructor
         *
         * \param[in] other guard object to copy.
         */
        guard(const guard &other) noexcept = default;

        /*!
         * \brief Move constructor
         *
         * \param[in] rhs guard object to copy.
         */
        guard(guard &&rhs) noexcept = default;

        /*!
         * \brief Copy assignment operator
         *
         * \param[in] other guard object to copy
         */
        guard& operator=(const guard &other) noexcept = default;

        /*!
         * \brief Move assignment operator
         *
         * \param[in] rhs guard object to copy
         */
        guard& operator=(guard &&rhs) noexcept = default;

        /*!
         * \brief Destroys the guard
         */
        virtual ~guard() noexcept { }

    protected:
        /*!
         * \brief Enables the guard with an alt.
         *
         * Called by an alt when the guard is being enabled during a select
         * operation.  Internally calls enable on the internal guard object.
         *
         * \param[in] a The alt being used during the select operation
         *
         * \return True if the guard is ready, false otherwise.
         */
        bool enable(const alt &a) const noexcept(false) { return _internal->enable(a); }

        /*!
         * \brief Disables the guard with an alt.
         *
         * Called by an alt when the guard is being disabled during a select
         * operation.  Internally calls disable on the internal guard object.
         *
         * \return True if the guard is ready, false otherwise.
         */
        bool disable() const noexcept(false) { return _internal->disable(); }

    };
}

#endif //CPP_CSP_GUARD_H
