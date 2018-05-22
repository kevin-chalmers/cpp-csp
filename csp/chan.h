//
// Created by kevin on 17/04/16.
//

#ifndef CPP_CSP_CHAN_H
#define CPP_CSP_CHAN_H

#include <memory>
#include <limits>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <boost/fiber/all.hpp>
#include "poison_exception.h"
#include "alt.h"
#include "alting_barrier.h"
#include "chan_data_store.h"

namespace csp
{
    // Forward declarations
    template<typename T, bool POISONABLE>
    class chan_in;
    template<typename T, bool POISONABLE>
    class chan_out;
    template<typename T, bool POISONABLE>
    class alting_chan_in;
    template<typename T, bool POISONABLE>
    class one2one_chan;
    template<typename T, bool POISONABLE>
    class fiber_one2one_chan;
    template<typename T, bool POISONABLE>
    class busy_one2one_chan;
    template<typename T, bool POISONABLE>
    class one2any_chan;
    template<typename T, bool POISONABLE>
    class fiber_one2any_chan;
    template<typename T, bool POISONABLE>
    class busy_one2any_chan;
    template<typename T, bool POISONABLE>
    class any2one_chan;
    template<typename T, bool POISONABLE>
    class fiber_any2one_chan;
    template<typename T, bool POISONABLE>
    class busy_any2one_chan;
    template<typename T, bool POISONABLE>
    class any2any_chan;
    template<typename T, bool POISONABLE>
    class fiber_any2any_chan;
    template<typename T, bool POISONABLE>
    class busy_any2any_chan;

    /*! \class chan
     * \brief A channel object.
     *
     * \tparam T Type that the channel operates on.
     *
     * \author Kevin Chalmers
     *
     * \date 18/04/2016
     */
    template<typename T, bool POISONABLE>
    class chan : public guard
    {
        friend class chan_in<T, POISONABLE>;
        friend class chan_out<T, POISONABLE>;
        friend class alting_chan_in<T, POISONABLE>;
    protected:
        /*! \class chan_internal
         * \brief Internal representation of a channel object.
         *
         * \author Kevin Chalmers
         *
         * \date 02/06/2016
         */
        class chan_internal : public guard::guard_internal
        {
        public:
            /*!
             * \brief Performs a write operation on the channel.
             *
             * \param[in] value Value to write to the channel.
             */
            virtual void write(T value) noexcept(false) = 0;

            /*!
             * \brief Performs a read operation on the channel.
             *
             * \return Value read from the channel
             */
            virtual T read() noexcept(false) = 0;

            /*!
            * \brief Starts an extended read operation.
            *
            * \return Value read from the channel.
            */
            virtual T start_read() noexcept(false) = 0;

            /*!
             * \brief Ends an extended read operation.
             */
            virtual void end_read() noexcept(false) = 0;

            /*!
             * \brief Checks if a message is pending on the channel.
             *
             * \return True if a message is ready on the channel, false otherwise.
             */
            virtual bool pending() const noexcept = 0;

            /*!
             * \brief Poisons the reading end of the channel.
             *
             * \param[in] strength The strength of the poison being used on the channel.
             */
            virtual void reader_poison(unsigned int strength) noexcept = 0;

            /*!
             * \brief Poisons the writing end of the channel.
             *
             * \param[in] strength The strength of the poison being used on the channel.
             */
            virtual void writer_poison(unsigned int strength) noexcept = 0;

            /*!
             * \brief Destroys the channel.
             */
            virtual ~chan_internal() { }
        };

        std::shared_ptr<chan_internal> _internal = nullptr; //<! Pointer to the internal representation of the channel.

        /*!
         * \brief Performs a write operation on the channel.
         *
         * \param[in] value Value to write to the channel.
         */
        void write(T value) const noexcept(false) { _internal->write(std::move(value)); }

        /*!
         * \brief Performs a read operation on the channel.
         *
         * \return Value read from the channel.
         */
        T read() const noexcept(false) { return _internal->read(); }

        /*!
         * \brief Starts an extended read operation.
         *
         * \return Value read from the channel.
         */
        T start_read() const noexcept(false) { return _internal->start_read(); }

        /*!
         * \brief Ends an extended read operation.
         */
        void end_read() const noexcept(false) { _internal->end_read(); }

        /*!
         * \brief Checks if a message is pending on the channel.
         *
         * \return True if a message is ready on the channel, false otherwise.
         */
        bool pending() const noexcept { return _internal->pending(); }

        /*!
         * \brief Poisons the reading end of the channel.
         *
         * \param[in] strength The strength of the poison being used on the channel.
         */
        void reader_poison(unsigned int strength) const noexcept { return _internal->reader_poison(strength); }

        /*!
         * \brief Poisons the writing end of the channel.
         *
         * \param[in] strength The strength of the poison being used on the channel.
         */
        void writer_poison(unsigned int strength) const noexcept { return _internal->writer_poison(strength); }

        /*!
         * \brief Protected constructor.  Used internally by child classes to create a channel object.
         */
        chan(std::shared_ptr<chan_internal> internal) noexcept
        : guard(internal), _internal(internal)
        {
        }

    public:

        /*!
         * \brief Copy constructor.
         *
         * \param[in] other Channel object to copy.
         */
        chan(const chan &other) noexcept = default;

        /*!
         * \brief Move constructor.
         *
         * \param[in] rhs Channel object to copy.
         */
        chan(chan &&rhs) noexcept = default;

        /*!
         * \brief Copy assignment operator.
         *
         * \param[in] other Channel object to copy.
         */
        chan& operator=(const chan &other) noexcept = default;

        /*!
         * \brief Move assignment operator.
         *
         * \param[in] rhs Channel object to copy.
         */
        chan& operator=(chan &&rhs) noexcept = default;

        /*!
         * \brief Virtual destructor - interface class.
         */
        virtual ~chan() noexcept { }
    };

    /*! \class chan_in
     * \brief The input end of a channel
     *
     * \tparam T Type that the channel operates on.
     * \tparam POISONABLE Flag used to indicate if the channel is poisonable.
     *
     * \author Kevin Chalmers
     *
     * \date 17/04/2016
     */
    template<typename T, bool POISONABLE = false>
    class chan_in
    {
    protected:
        /*! \class chan_in_internal
         * \brief Internal representation of the input end of a channel.
         *
         * \author Kevin Chalmers
         *
         * \date 17/04/2016
         */
        class chan_in_internal
        {
        protected:
            chan<T, POISONABLE> _chan; //<! Pointer to the internal channel implementation.

            unsigned int _immunity = std::numeric_limits<unsigned int>::max();

        public:
            /*!
             * \brief Creates a new internal channel input
             *
             * \param[in] chan The internal representation of the channel.
             * \param[in] immunity The poison immunity level.
             */
            chan_in_internal(chan<T, POISONABLE> chan, unsigned int immunity) noexcept
            : _chan(chan), _immunity(immunity)
            {
            }

            /*!
             * \brief Virtual destructor.
             */
            virtual ~chan_in_internal() noexcept { }

            /*!
             * \brief Read operation.
             *
             * \return The value read from the channel.
             */
            virtual T read() const noexcept(false) { return _chan.read(); }

            /*!
             * \brief Begins an extended read operation.
             *
             * \return The value read from the channel.
             */
            virtual T start_read() const noexcept(false) { return _chan.start_read(); }

            /*!
             * \brief Ends an extended read operation.
             */
            virtual void end_read() const noexcept(false) { _chan.end_read(); }

            /*!
             * \brief Poisons the channel end.
             *
             * \param strength The strength of the poison to apply to the channel.
             */
            virtual void poison(unsigned int strength) const noexcept { _chan.reader_poison(strength); }
        };

        std::shared_ptr<chan_in_internal> _internal = nullptr; //<! Pointer to the internal representation.

        /*!
         * \brief Creates a new channel input end from an existing internal channel input.
         *
         * \param[in] internal Pointer to the internal representation of the channel.
         */
        chan_in(std::shared_ptr<chan_in_internal> internal) noexcept
        : _internal(internal)
        {
        }

    public:
        /*!
         * \brief Copy constructor
         *
         * \param[in] other The channel input end to copy.
         */
        chan_in(const chan_in<T, POISONABLE> &other) noexcept = default;

        /*!
         * \brief Move constructor
         *
         * \param[in] rhs The channel input end to copy.
         */
        chan_in(chan_in<T, POISONABLE> &&rhs) noexcept = default;

        /*!
         * \brief Copy assignment operator.
         *
         * \param[in] other The channel input end to copy.
         *
         * \return The copied channel input end.
         */
        chan_in<T, POISONABLE>& operator=(const chan_in<T, POISONABLE> &other) noexcept = default;

        /*!
         * \brief Move assignment operator.
         *
         * \param[in] rhs The channel input end to copy.
         *
         * \return The copied channel input end.
         */
        chan_in<T, POISONABLE>& operator=(chan_in<T, POISONABLE> &&rhs) noexcept = default;

        /*!
         * \brief Virtual destructor.  Interface class.
         */
        virtual ~chan_in() noexcept { }

        /*!
         * \brief Equality operator.  Checks the pointer to the internal channel input.
         *
         * \param[in] other The channel input end to compare to.
         *
         * \return True if the shared pointers are equal, false otherwise.
         */
        bool operator==(const chan_in<T, POISONABLE> &other) const noexcept { return this->_internal == other._internal; }

        /*!
         * \brief Inequality operator.  Checks the pointer to the internal channel input.
         *
         * \param[in] other The channel input end to compare to.
         *
         * \return True if the share pointers are not equal, false otherwise.
         */
        bool operator!=(const chan_in<T, POISONABLE> &other) const noexcept { return this->_internal != other._internal; }

        /*!
         * \brief Reads a value from the channel.
         *
         * \return Value read from the channel.
         */
        T read() const noexcept(false) { return _internal->read(); }

        /*!
         * \brief Operator overload to read a value from the channel.
         *
         * \return Value read from the channel.
         */
        T operator()() const noexcept(false) { return _internal->read(); }

        /*!
         * \brief Begins an extended read operation.
         *
         * \return Value read from the channel.
         */
        T start_read() const noexcept(false) { return _internal->start_read(); }

        /*!
         * \brief Ends an extended read operation.
         */
        void end_read() const noexcept(false) { _internal->end_read(); }

        /*!
         * \brief Poisons the channel.
         *
         * \param[in] strength The strength of the poison to use on the channel.
         */
        void poison(unsigned int strength) const noexcept
        {
            static_assert(POISONABLE, "channel is not poisonable");
            _internal->poison(strength);
        }
    };

    /*! \class shared_chan_in
     * \brief A shared input end of a channel
     *
     * \tparam T Type that the channel operates on.
     * \tparam POISONABLE Flag to indicate whether this channel can be poisoned.
     *
     * \author Kevin Chalmers
     *
     * \date 18/04/2016
     */
    template<typename T, bool POISONABLE = false>
    class shared_chan_in : public chan_in<T, POISONABLE>
    {
        // Friend declarations
        friend class one2any_chan<T, POISONABLE>;
        friend class any2any_chan<T, POISONABLE>;
    protected:
        /*! \class shared_chan_in_internal
         * \brief Internal representation of a shared channel input.
         *
         * \author Kevin Chalmers
         *
         * \date 18/04/2016
         */
        class shared_chan_in_internal : public chan_in<T, POISONABLE>::chan_in_internal
        {
        public:

            mutable std::mutex _mut; //<! Mutex used to control access to the channel.

            /*!
             * \brief Creates a new internal shared channel input.
             *
             * \param[in] chan Pointer to the actual channel.
             * \param[in] immunity The poison immunity level.
             */
            shared_chan_in_internal(chan<T, POISONABLE> chan, unsigned int immunity) noexcept
            : chan_in<T, POISONABLE>::chan_in_internal(chan, immunity)
            {
            }

            /*!
             * \brief Reads a value from the channel.
             *
             * \return Value read from the channel.
             */
            T read() const noexcept(false) override
            {
                // Lock the channel
                std::unique_lock<std::mutex> lock(_mut);
                // Perform the read
                return chan_in<T, POISONABLE>::chan_in_internal::read();
            }

            /*!
             * \brief Starts an extended read on the channel.
             *
             * \return Value read from the channel.
             */
            T start_read() const noexcept(false) override
            {
                // Lock the channel
                _mut.lock();
                // Perform the extended read
                return chan_in<T, POISONABLE>::chan_in_internal::start_read();
            }

            /*!
             * \brief Ends an extended read on the channel.
             */
            void end_read() const noexcept(false) override
            {
                // End the read
                chan_in<T, POISONABLE>::chan_in_internal::end_read();
                // Unlock the channel
                _mut.unlock();
            }

            /*!
             * \brief Poisons the channel.
             *
             * \param[in] strength The strength of the poison to apply to the channel.
             */
            void poison(unsigned int strength) const noexcept override
            {
                // Lock the channel
                std::unique_lock<std::mutex> lock(_mut);
                // Poison the channel
                chan_in<T, POISONABLE>::chan_in_internal::poison(strength);
            }
        };

        std::shared_ptr<shared_chan_in_internal> _internal = nullptr; //<! Pointer to the internal representation of the shared channel input.

        /*!
         * \brief Creates a new shared channel input from an existing internal representation.
         *
         * \param[in] internal Pointer to the internal representation of the shared channel input.
         */
        shared_chan_in(std::shared_ptr<shared_chan_in_internal> internal) noexcept
        : chan_in<T, POISONABLE>(internal), _internal(internal)
        {
        }

    public:
        /*!
         * \brief Copy constructor.
         *
         * \param[in] other The shared channel input to copy.
         */
        shared_chan_in(const shared_chan_in<T, POISONABLE> &other) noexcept = default;

        /*!
         * \brief Move constructor.
         *
         * \param[in] rhs The shared channel input to copy.
         */
        shared_chan_in(shared_chan_in<T, POISONABLE> &&rhs) noexcept = default;

        /*!
         * \brief Copy assignment operator.
         *
         * \param[in] other The shared channel input to copy.
         *
         * \return A copy of the shared channel input.
         */
        shared_chan_in& operator=(const shared_chan_in<T, POISONABLE> &other) noexcept = default;

        /*!
         * \brief Move assignment operator.
         *
         * \param[in] rhs The shared channel input to copy.
         *
         * \return A copy of the shared channel input.
         */
        shared_chan_in& operator=(shared_chan_in<T, POISONABLE> &&rhs) noexcept = default;
    };

    template<typename T, bool POISONABLE=false>
    class fiber_shared_chan_in : public shared_chan_in<T, POISONABLE>
    {
        friend class fiber_one2any_chan<T, POISONABLE>;
        friend class fiber_any2any_chan<T, POISONABLE>;
    protected:
        class fiber_shared_chan_in_internal : public shared_chan_in<T, POISONABLE>::internal
        {
        public:
            mutable boost::fibers::mutex _mut;

            fiber_shared_chan_in_internal(chan<T, POISONABLE> chan, unsigned int immunity) noexcept
            : shared_chan_in<T, POISONABLE>::shared_chan_in_internal(chan, immunity)
            {
            }

            T read() const final
            {
                std::unique_lock<boost::fibers::mutex> lock(_mut);
                return chan_in<T, POISONABLE>::chan_in_internal::read();
            }

            T start_read() const final
            {
                _mut.lock();
                return chan_in<T, POISONABLE>::chan_in_internal::start_read();
            }

            void end_read() const final
            {
                chan_in<T, POISONABLE>::chan_in_internal::end_read();
                _mut.unlock();
            }
        };

        std::shared_ptr<fiber_shared_chan_in_internal> _internal = nullptr;

        fiber_shared_chan_in(std::shared_ptr<fiber_shared_chan_in_internal> internal) noexcept
        : shared_chan_in<T, POISONABLE>(internal), _internal(internal)
        {
        }

    public:
        fiber_shared_chan_in(const fiber_shared_chan_in<T, POISONABLE>&) = default;

        fiber_shared_chan_in(fiber_shared_chan_in<T, POISONABLE>&&) = default;

        fiber_shared_chan_in& operator=(const fiber_shared_chan_in<T, POISONABLE>&) = default;

        fiber_shared_chan_in& operator=(fiber_shared_chan_in<T, POISONABLE>&&) = default;
    };

    /*! \class busy_shared_chan_in
     * \brief A shared input end using an atomic flag to create a busy wait.
     *
     * \tparam T The type that the channel operates on.
     * \tparam POISONABLE Flag used to indicate if the channel is poisonable.
     *
     * \author Kevin Chalmers
     *
     * \date 18/10/2016
     */
    template<typename T, bool POISONABLE = false>
    class busy_shared_chan_in : public shared_chan_in<T, POISONABLE>
    {
        // Friend declarations.
        friend class busy_one2any_chan<T, POISONABLE>;
        friend class busy_any2any_chan<T, POISONABLE>;
    protected:
        /*! \class busy_shared_chan_in_internal
         * \brief Internal implementation of busy shared_chan_in.
         *
         * \author Kevin Chalmers
         *
         * \date 18/10/2016
         */
        class busy_shared_chan_in_internal : public shared_chan_in<T, POISONABLE>::shared_chan_in_internal
        {
        public:
            mutable std::atomic_flag _flag = ATOMIC_FLAG_INIT;

            /*!
             * \brief Creates a new internal busy shared chan in.
             *
             * \param[in] chan Pointer to the actual channel.
             * \param[in] immunity The poison immunity level.
             */
            busy_shared_chan_in_internal(chan<T, POISONABLE> chan, unsigned int immunity) noexcept
            : shared_chan_in<T, POISONABLE>::shared_chan_in_internal(chan, immunity)
            {
            }

            /*!
             * \brief Reads a value from the channel.
             *
             * \return Value read from the channel.
             */
            T read() const noexcept(false) override
            {
                // Spin trying to claim the flag.
                while (!_flag.test_and_set(std::memory_order_acquire));
                // Get the value by performing the actual read.
                auto val = chan_in<T, POISONABLE>::chan_in_internal::read();
                // Clear the flag, allowing next reader to proceed.
                _flag.clear(std::memory_order_release);
                // Return the value
                return std::move(val);
            }

            /*!
             * \brief Starts an extended read on the channel.
             *
             * \return Value read from the channel.
             */
            T start_read() const noexcept(false) override
            {
                // Spin trying to claim the flag.
                while (!_flag.test_and_set(std::memory_order_acquire));
                // Just return the read value - we don't clear the flag here.
                return chan_in<T, POISONABLE>::chan_in_internal::start_read();
            }

            /*!
             * \brief Completed an extended read on the channel.
             */
            void end_read() const noexcept(false) override
            {
                // End the read.
                chan_in<T, POISONABLE>::chan_in_internal::end_read();
                // And clear the flag
                _flag.clear(std::memory_order_release);
            }

            /*!
             * \brief Poisons the channel.
             *
             * \param[in] strength The strength of the poison to apply to the channel.
             */
            void poison(unsigned int strength) const noexcept override
            {
                // Spin trying to claim the flag.
                while (!_flag.test_and_set(std::memory_order_acquire));
                // Call poison on the channel.
                chan_in<T, POISONABLE>::chan_in_internal::poison(strength);
                // Clear the flag
                _flag.clear(std::memory_order_release);
            }
        };

        std::shared_ptr<busy_shared_chan_in_internal> _internal = nullptr; //<! Pointer to the internal representation of the busy shared channel input.

        /*!
         * \brief Creates a new busy shared channel input from an existing internal representation.
         *
         * \param[in] internal Pointer to the internal representation of the shared channel input.
         */
        busy_shared_chan_in(std::shared_ptr<busy_shared_chan_in_internal> internal) noexcept
        : shared_chan_in<T, POISONABLE>(internal), _internal(internal)
        {
        }

    public:
        /*!
         * \brief Copy constructor.
         *
         * \param[in] other The shared channel input to copy.
         */
        busy_shared_chan_in(const busy_shared_chan_in<T, POISONABLE> &other) noexcept = default;

        /*!
         * \brief Move constructor.
         *
         * \param[in] rhs The shared channel input to copy.
         */
        busy_shared_chan_in(busy_shared_chan_in<T, POISONABLE> &&rhs) noexcept = default;

        /*!
         * \brief Copy assignment operator.
         *
         * \param[in] other The shared channel input to copy.
         *
         * \return A copy of the shared channel input.
         */
        busy_shared_chan_in& operator=(const busy_shared_chan_in<T, POISONABLE> &other) noexcept = default;

        /*!
         * \brief Move assignment operator.
         *
         * \param[in] rhs The shared channel input to copy.
         *
         * \return A copy of the shared channel input.
         */
        busy_shared_chan_in& operator=(busy_shared_chan_in<T, POISONABLE> &&rhs) noexcept = default;
    };

    /*! \class alting_chan_in
     * \brief A guarded input end of a channel.
     *
     * \tparam T The type that the channel operates on.
     * \tparam POISONABLE Flag used to indicate if the channel is poisonable.
     *
     * \author Kevin Chalmers
     *
     * \date 18/04/2016
     */
    template<typename T, bool POISONABLE = false>
    class alting_chan_in : public chan_in<T, POISONABLE>, public guard
    {
        // Friend declarations
        friend class one2one_chan<T, POISONABLE>;
        friend class busy_one2one_chan<T, POISONABLE>;
        friend class fiber_one2one_chan<T, POISONABLE>;
        friend class any2one_chan<T, POISONABLE>;
        friend class busy_any2one_chan<T, POISONABLE>;
    protected:
        /*! \class alting_chan_in_internal.
         * \brief Creates a new internal alting channel input.
         *
         * \author Kevin Chalmers
         *
         * \date 18/04/2016
         */
        class alting_chan_in_internal : public chan_in<T, POISONABLE>::chan_in_internal, public guard::guard_internal
        {
        public:
            /*!
             * \brief Creates a new internal alting channel input.
             *
             * \param[in] chan The internal channel object.
             * \param[in] immunity The immunity of the channel.
             */
            alting_chan_in_internal(chan<T, POISONABLE> chan, unsigned int immunity) noexcept
            : chan_in<T, POISONABLE>::chan_in_internal(chan, immunity)
            {
            }

            /*!
             * \brief Checks if a message is pending on the channel.
             *
             * \return True if a value is ready on the channel, false otherwise.
             */
            virtual bool pending() const noexcept
            {
                return chan_in<T, POISONABLE>::chan_in_internal::_chan.pending();
            }

            /*!
             * \brief Enables the channel in an alt selection
             *
             * \param[in] a The alt used in the selection.
             *
             * \return True if the channel is ready, false otherwise.
             */
            bool enable(const alt &a) noexcept(false) override
            {
                return chan_in<T, POISONABLE>::chan_in_internal::_chan.enable(a);
            }

            /*!
             * \brief Disables the channel in an alt selection.
             *
             * \return True if the channel is ready, false otherwise.
             */
            bool disable() noexcept override
            {
                return chan_in<T, POISONABLE>::chan_in_internal::_chan.disable();
            }
        };

        std::shared_ptr<alting_chan_in_internal> _internal = nullptr; //<! Pointer to the internal representation of the alting channel input.

        /*!
         * \brief Creates a new alting channel input from an existing internal representation.
         *
         * \param[in] internal Pointer to the internal representation of the alting channel input.
         */
        alting_chan_in(std::shared_ptr<alting_chan_in_internal> internal) noexcept
        : chan_in<T, POISONABLE>(internal), guard(internal), _internal(internal)
        {
        }

    public:
        /*!
         * \brief Copy constructor.
         *
         * \param[in] other The alting channel input to copy.
         */
        alting_chan_in(const alting_chan_in<T, POISONABLE> &other) noexcept = default;

        /*!
         * \brief Move constructor.
         *
         * \param[in] rhs The alting channel input to copy.
         */
        alting_chan_in(alting_chan_in<T, POISONABLE> &&rhs) noexcept = default;

        /*!
         * \brief Copy assignment operator.
         *
         * \param[in] other The alting channel input to copy.
         *
         * \return A copy of the alting channel input.
         */
        alting_chan_in& operator=(const alting_chan_in<T, POISONABLE> &other) noexcept = default;

        /*!
         * \brief Move assignment operator.
         *
         * \param[in] rhs The alting channel input to copy.
         *
         * \return A copy of the alting channel input.
         */
        alting_chan_in& operator=(alting_chan_in<T, POISONABLE> &&rhs) noexcept = default;

        /*!
         * \brief Checks if a message is waiting on the channel.
         *
         * \return True if the channel is ready, false otherwise.
         */
        bool pending() const noexcept
        {
            return _internal->pending();
        }
    };

    /*! \class chan_out
     * \brief An output end of a channel.
     *
     * \tparam T The type that the channel operates on.
     * \tparam POISONABLE Flag to indicate if the channel can be poisoned.
     *
     * \author Kevin Chalmers
     *
     * \date 19/04/2016
     */
    template<typename T, bool POISONABLE = false>
    class chan_out
    {
        // Friend declarations
        friend class one2one_chan<T, POISONABLE>;
        friend class fiber_one2one_chan<T, POISONABLE>;
        friend class busy_one2one_chan<T, POISONABLE>;
        friend class one2any_chan<T, POISONABLE>;
    protected:
        /*! \class chan_out_internal
         * \brief Internal representation of a channel output.
         *
         * \author Kevin Chalmers
         *
         * \date 19/04/2016
         */
        class chan_out_internal
        {
        public:

            unsigned int _immunity = std::numeric_limits<unsigned int>::max(); //<! The immunity level of the channel.

            chan<T, POISONABLE> _chan;

            /*!
             * \brief Creates a new internal channel output.
             *
             * \param[in] chan Pointer to the internal channel representation.
             * \param[in] immunity The poison immunity level of the channel.
             */
            chan_out_internal(chan<T, POISONABLE> chan, unsigned int immunity) noexcept
            : _chan(chan), _immunity(immunity)
            {
            }

            /*!
             * \brief Writes a value to the channel.
             *
             * \param[in] value Value to write to the channel.
             */
            virtual void write(T value) const noexcept(false) { _chan.write(std::move(value)); }

            /*!
             * \brief Poisons the channel.
             *
             * \param[in] strength The strength of the poison to use on the channel.
             */
            virtual void poison(unsigned int strength) const noexcept { _chan.writer_poison(strength); }

        public:

            /*!
             * \brief Destroys the channel
             */
            virtual ~chan_out_internal() noexcept { }
        };

        std::shared_ptr<chan_out_internal> _internal = nullptr; //<! Pointer to the internal representation of the channel.

        /*!
         * \brief Protected constructor.  Used to create a chan_out from an existing internal representation.
         *
         * \param[in] internal Pointer to the internal representation of the chan_out.
         */
        chan_out(std::shared_ptr<chan_out_internal> internal) noexcept
        : _internal(internal)
        {
        }

    public:
        /*!
         * \brief Copy constructor.
         *
         * \param[in] other The chan_out to copy.
         */
        chan_out(const chan_out<T, POISONABLE> &other) noexcept = default;

        /*!
         * \brief Move constructor.
         *
         * \param[in] rhs The chan_out to copy.
         */
        chan_out(chan_out<T, POISONABLE> &&rhs) noexcept = default;

        /*!
         * \brief Virtual destructor.  Base class.
         */
        virtual ~chan_out() noexcept { }

        /*!
         * \brief Copy assignment operator.
         *
         * \param[in] other The chan_out to copy.
         *
         * \return A copy of the chan_out.
         */
        chan_out<T, POISONABLE>& operator=(const chan_out<T, POISONABLE> &other) noexcept = default;

        /*!
         * \brief Move assignment operator.
         *
         * \param[in] rhs The chan_out to copy.
         *
         * \return A copy of the chan_out.
         */
        chan_out<T, POISONABLE>& operator=(chan_out<T, POISONABLE> &&rhs) noexcept = default;

        /*!
         * \brief Equality operator.  Compares the internal shared pointer.
         *
         * \param[in] other The chan_out to compare to.
         *
         * \return True if the internal shared pointers are the same, false otherwise.
         */
        bool operator==(const chan_out<T, POISONABLE> &other) const noexcept { return this->_internal == other._internal; }

        /*!
         * \brief Inequality operator.  Compares the internal shared pointer.
         *
         * \param[in] other The chan_out to compare to.
         *
         * \return True if the internal shared pointers are not the same, false otherwise.
         */
        bool operator!=(const chan_out<T, POISONABLE> &other) const noexcept { return this->_internal == other._internal; }

        /*!
         * \brief Less than operator.  Compares the internal shared pointer.
         *
         * \param[in] other The chan_out to compare to.
         *
         * \return True if the internal shared pointer is less than the other, false otherwise.
         */
        bool operator<(const chan_out<T, POISONABLE> &other) const noexcept { return this->_internal < other._internal; }

        /*!
         * \brief Greater than operator.  Compares the internal shared pointer.
         *
         * \param[in] other The chan_out to compare to.
         *
         * \return True if the internal shared pointer is greater than the other, false otherwise.
         */
        bool operator>(const chan_out<T, POISONABLE> &other) const noexcept { return this->_internal > other._internal; }

        /*!
         * \brief Less than or equal to operator.  Compares the internal shared pointer.
         *
         * \param[in] other The chan_out to compare to.
         *
         * \return True if the internal shared pointer is less than or equal to the other, false otherwise.
         */
        bool operator<=(const chan_out<T, POISONABLE> &other) const noexcept { return this->_internal <= other._internal; }

        /*!
         * \brief Greater than or equal to operator.  Compare the internal shared pointer.
         *
         * \param[in] other The chan_out to compare to.
         *
         * \return True if the internal shared pointer is greater than or equal to the other, false otherwise.
         */
        bool operator>=(const chan_out<T, POISONABLE> &other) const noexcept { return this->_internal >= other._internal; }

        /*!
         * \brief Writes a value to the channel.
         *
         * \param[in] value Value to write to the channel.
         */
        void write(T value) const noexcept { _internal->write(std::move(value)); }

        /*!
         * \brief Operator overload to write a value to the channel.
         *
         * \param[in] value Value to write to the channel.
         */
        void operator()(T value) const noexcept { _internal->write(std::move(value)); }

        /*!
         * \brief Poisons the channel.
         *
         * \param[in] strength The strength of the poison to apply to the channel.
         */
        void poison(unsigned int strength) const noexcept { _internal->poison(strength); }
    };

    /*! \class shared_chan_out
     * \brief A shared output end of a channel
     *
     * \tparam T Type that the channel operates on.
     * \tparam POISONABLE Flag to indicate whether this channel can be poisoned.
     *
     * \author Kevin Chalmers
     *
     * \date 22/04/2016
     */
    template<typename T, bool POISONABLE = false>
    class shared_chan_out : public chan_out<T, POISONABLE>
    {
        // Friend declarations
        friend class any2one_chan<T, POISONABLE>;
        friend class any2any_chan<T, POISONABLE>;
    protected:
        /*! \class shared_chan_out_internal
         * \brief Internal representation of a shared channel output.
         *
         * \author Kevin Chalmers
         *
         * \date 22/04/2016
         */
        class shared_chan_out_internal : public chan_out<T, POISONABLE>::chan_out_internal
        {
        public:

            mutable std::mutex _mut; //<! Mutex used to control access to the channel.

            /*!
             * \brief Creates a new internal shared channel output from an existing pointer to a channel.
             *
             * \param[in] chan Pointer to the internal channel object.
             * \param[in] immunity The poison immunity level of the channel.
             */
            shared_chan_out_internal(chan<T, POISONABLE> chan, unsigned int immunity) noexcept
            : chan_out<T, POISONABLE>::chan_out_internal(chan, immunity)
            {
            }

            /*!
             * \brief Writes a value to the channel.
             *
             * \param[in] value The value to write to the channel.
             */
            void write(T value) const noexcept override
            {
                // Lock the channel
                std::unique_lock<std::mutex> lock(_mut);
                // Perform the write
                chan_out<T, POISONABLE>::chan_out_internal::write(std::move(value));
            }

            /*!
             * \brief Poisons the channel.
             *
             * \param[in] strength The strength of the poison to apply to the channel.
             */
            void poison(unsigned int strength) const noexcept override
            {
                // Lock the channel
                std::unique_lock<std::mutex> lock(_mut);
                // Perform the write
                chan_out<T, POISONABLE>::chan_out_internal::poison(strength);
            }

        };

        /*!
         * \brief Creates a shared_chan_out from an existing pointer to an internal shared chan output.
         *
         * \param[in] internal Pointer to the internal representation of the channel.
         */
        shared_chan_out(std::shared_ptr<shared_chan_out_internal> internal) noexcept
        : chan_out<T, POISONABLE>(internal)
        {
        }

    public:

        /*!
         * \brief Copy constructor.
         *
         * \param[in] other The shared_chan_out to copy.
         */
        shared_chan_out(const shared_chan_out<T, POISONABLE> &other) noexcept = default;

        /*!
         * \brief Move constructor.
         *
         * \param[in] rhs The shared_chan_out to copy.
         */
        shared_chan_out(shared_chan_out<T, POISONABLE> &&rhs) noexcept = default;

        /*!
         * \brief Copy assignment operator.
         *
         * \param[in] other The shared_chan_out to copy.
         *
         * \return A copy of the shared_chan_out.
         */
        shared_chan_out<T, POISONABLE>& operator=(const shared_chan_out<T, POISONABLE> &other) noexcept = default;

        /*!
         * \brief Move assignment operator.
         *
         * \param[in] rhs The shared_chan_out to copy.
         *
         * \return A copy of the shared_chan_out.
         */
        shared_chan_out<T, POISONABLE>& operator=(shared_chan_out &&rhs) noexcept = default;
    };

    template<typename T, bool POISONABLE = false>
    class fiber_shared_chan_out : public shared_chan_out<T, POISONABLE>
    {
        friend class fiber_any2one_chan<T, POISONABLE>;
        friend class fiber_any2any_chan<T, POISONABLE>;
    protected:
        class fiber_shared_chan_out_internal : public shared_chan_out<T, POISONABLE>::internal
        {
        public:
            mutable boost::fibers::mutex _mut;

            fiber_shared_chan_out_internal(chan<T, POISONABLE> chan, unsigned int immunity)
            : shared_chan_out<T, POISONABLE>::shared_chan_out_internal(chan, immunity)
            {
            }

            void write(T value) const noexcept final
            {
                std::unique_lock<boost::fibers::mutex> lock(_mut);
                chan_out<T, POISONABLE>::chan_out_internal::write(std::move(value));
            }

            void poison(unsigned int strength) const noexcept final
            {
                std::unique_lock<boost::fibers::mutex> lock(_mut);
                chan_out<T, POISONABLE>::chan_out_internal::poison(strength);
            }
        };

        std::shared_ptr<fiber_shared_chan_out_internal> _internal = nullptr;

        fiber_shared_chan_out(std::shared_ptr<fiber_shared_chan_out_internal> internal)
        : shared_chan_out<T, POISONABLE>(internal), _internal(internal)
        {
        }

    public:
        fiber_shared_chan_out(const fiber_shared_chan_out<T, POISONABLE>&) = default;

        fiber_shared_chan_out(fiber_shared_chan_out<T, POISONABLE>&&) = default;

        fiber_shared_chan_out& operator=(const fiber_shared_chan_out<T, POISONABLE>&) = default;

        fiber_shared_chan_out& operator=(fiber_shared_chan_out<T, POISONABLE>&&) = default;
    };

    /*! \class busy_shared_chan_out
     * \brief A shared output end using an atomic flag to create a busy wait.
     *
     * \tparam T The type that the channel operates on.
     * \tparam POISONABLE Flag used to indicate if the channel is poisonable.
     *
     * \author Kevin Chalmers
     *
     * \date 19/10/2016
     */
    template<typename T, bool POISONABLE = false>
    class busy_shared_chan_out : public shared_chan_out<T, POISONABLE>
    {
        // Friend declarations.
        friend class busy_any2one_chan<T, POISONABLE>;
        friend class busy_any2any_chan<T, POISONABLE>;
    protected:
        /*! \class busy_shared_chan_out_internal
         * \brief Internal implementation of busy shared_chan_in.
         *
         * \author Kevin Chalmers
         *
         * \date 19/10/2016
         */
        class busy_shared_chan_out_internal : public shared_chan_out<T, POISONABLE>::shared_chan_out_internal
        {
        public:
            mutable std::atomic_flag _flag = ATOMIC_FLAG_INIT;

            /*!
             * \brief Creates a new internal busy shared chan out.
             *
             * \param[in] chan Pointer to the actual channel.
             * \param[in] immunity The poison immunity level.
             */
            busy_shared_chan_out_internal(chan<T, POISONABLE> chan, unsigned int immunity) noexcept
            : shared_chan_out<T, POISONABLE>::shared_chan_out_internal(chan, immunity)
            {
            }

            /*!
             * \brief Writes a value to the channel.
             */
            void write(T value) const noexcept override
            {
                // Spin trying to claim the flag.
                while (!_flag.test_and_set());
                // Write the value by performing the actual read.
                chan_out<T, POISONABLE>::chan_out_internal::write(std::move(value));
                // Clear the flag, allowing next writer to proceed.
                _flag.clear();
            }

            /*!
             * \brief Poisons the channel.
             *
             * \param[in] strength The strength of the poison to apply to the channel.
             */
            void poison(unsigned int strength) const noexcept override
            {
                // Spin trying to claim the flag.
                while (!_flag.test_and_set());
                // Call poison on the channel.
                chan_out<T, POISONABLE>::chan_out_internal::poison(strength);
                // Clear the flag
                _flag.clear();
            }
        };

        std::shared_ptr<busy_shared_chan_out_internal> _internal = nullptr; //<! Pointer to the internal representation of the busy shared channel input.

        /*!
         * \brief Creates a new busy shared channel output from an existing internal representation.
         *
         * \param[in] internal Pointer to the internal representation of the shared channel input.
         */
        busy_shared_chan_out(std::shared_ptr<busy_shared_chan_out_internal> internal) noexcept
        : shared_chan_out<T, POISONABLE>(internal), _internal(internal)
        {
        }

    public:
        /*!
         * \brief Copy constructor.
         *
         * \param[in] other The shared channel output to copy.
         */
        busy_shared_chan_out(const busy_shared_chan_out<T, POISONABLE> &other) noexcept = default;

        /*!
         * \brief Move constructor.
         *
         * \param[in] rhs The shared channel output to copy.
         */
        busy_shared_chan_out(busy_shared_chan_out<T, POISONABLE> &&rhs) noexcept = default;

        /*!
         * \brief Copy assignment operator.
         *
         * \param[in] other The shared channel output to copy.
         *
         * \return A copy of the shared channel output.
         */
        busy_shared_chan_out& operator=(const busy_shared_chan_out<T, POISONABLE> &other) noexcept = default;

        /*!
         * \brief Move assignment operator.
         *
         * \param[in] rhs The shared channel output to copy.
         *
         * \return A copy of the shared channel output.
         */
        busy_shared_chan_out& operator=(busy_shared_chan_out<T, POISONABLE> &&rhs) noexcept = default;
    };

    /*! \class alting_chan_out
     * \brief A guarded output end of a channel.
     *
     * \tparam T Type that the channel operates on.
     * \tparam POISONABLE Flag to indicate whether this channel can be poisoned.
     *
     * \author Kevin Chalmers
     *
     * \date 22/04/2016
     */
    template<typename T, bool POISONABLE = false>
    class alting_chan_out : public chan_out<T, POISONABLE>, public guard
    {
    protected:
        /*! \class alting_chan_out_internal
         * \brief Internal representation of an alting_chan_out
         *
         * \author Kevin Chalmers
         *
         * \date 25/04/2016
         */
        class alting_chan_out_internal : public chan_out<T, POISONABLE>::chan_out_internal, public guard_internal
        {
        protected:
            /*!
             * \brief Creates a new alting_chan_out_internal
             *
             * \param[in] chan The internal channel object.
             * \param[in] immunity The poison immunity level of the channel.
             */
            alting_chan_out_internal(std::shared_ptr<chan<T, POISONABLE>> chan, unsigned int immunity) noexcept
            : chan_out<T, POISONABLE>::chan_out_internal(chan, immunity)
            {
            }
        };

        std::shared_ptr<alting_chan_out_internal> _internal = nullptr; //!< Pointer to the internal representation of the channel

        /*!
         * \brief Creates a new alting_chan_out from a pointer to an existing internal channel object.
         *
         * \param[in] internal Pointer to the internal channel representation.
         */
        alting_chan_out(std::shared_ptr<alting_chan_out_internal> internal) noexcept
        : chan_out<T, POISONABLE>(internal), guard(internal), _internal(internal)
        {
        }

    public:
        /*!
         * \brief Copy constructor.
         *
         * \param[in] other The alting_chan_out to copy.
         */
        alting_chan_out(const alting_chan_out<T, POISONABLE> &other) noexcept = default;

        /*!
         * \brief Move constructor.
         *
         * \param[in] rhs The alting_chan_out to copy.
         */
        alting_chan_out(alting_chan_out<T, POISONABLE> &&rhs) noexcept = default;

        /*!
         * \brief Copy assignment operator.
         *
         * \param[in] other The alting_chan_out to copy.
         *
         * \return A copy of the alting_chan_out.
         */
        alting_chan_out<T, POISONABLE>& operator=(const alting_chan_out<T, POISONABLE> &other) noexcept = default;

        /*!
         * \brief Move assignment operator.
         *
         * \param[in] rhs The alting_chan_out to copy.
         *
         * \return A copy of the alting_chan_out.
         */
        alting_chan_out<T, POISONABLE>& operator=(alting_chan_out<T, POISONABLE> &&rhs) noexcept = default;
    };

    // TODO: alting channel symmetrics

    /*! \class chan_impl
     * \brief Standard implementation of a channel.  Used internally to the framework.
     *
     * \tparam T The type that the channel operates on.
     * \tparam POISONABLE Flag used to indicate if the channel can be poisoned.
     *
     * \author Kevin Chalmers
     *
     * \date 25/04/2016
     */
    template<typename T, bool POISONABLE = false>
    class basic_chan : public chan<T, POISONABLE>
    {
    protected:
        /*! \class basic_chan_internal
         * \brief Internal representation of a basic channel.
         *
         * \author Kevin Chalmers
         *
         * \date 02/06/2016
         */
        class basic_chan_internal : public chan<T, POISONABLE>::chan_internal
        {
        private:

            mutable std::mutex _mut; //!< Lock used to control access to the channel.

            std::condition_variable _cond; //!< Condition variable used to wait for events.

            std::vector<T> _hold; //!< Current value on the channel.

            bool _reading = false; //!< Flag used to determine when the channel is in an extended read state.

            bool _empty = true; //!< Flag used to indicate whether the channel is empty or not.

            alt _alt; //!< Alt used when channel is in a selection operation.

            bool _alting = false; //!< Flag used to indicate whether the channel is being used in a selection operation.

            unsigned int _strength = 0; //!< Strength of poison on channel.

        protected:
            /*!
             * \brief Performs a write operation on the channel.
             *
             * \param[in] value The value to write to the channel.
             */
            void write(T value) noexcept(false) override final
            {
                // Lock the mutex
                std::unique_lock<std::mutex> lock(_mut);

                // Check if poisoned
                if (_strength > 0)
                    throw poison_exception(_strength);
                // Put the value in the hold
                _hold.push_back(std::move(value));
                // If channel is empty, then set empty to false and notify any waiting alt.
                if (_empty)
                {
                    _empty = false;
                    if (_alting)
                        guard::guard_internal::schedule(_alt);
                }
                else
                {
                    // Otherwise complete the communication
                    _empty = true;
                    _cond.notify_one();
                }
                // Wait until reader has completed
                _cond.wait(lock);
                // Check if poisoned
                if (_strength > 0)
                    throw poison_exception(_strength);
            }

            /*!
             * \brief Performs a read operation on the channel.
             *
             * \return The value read from the channel.
             */
            T read() noexcept(false) override final
            {
                // Lock the mutex
                std::unique_lock<std::mutex> lock(_mut);
                // Check if poisoned
                if (_strength > 0)
                    throw poison_exception(_strength);
                // Check if channel is empty, and if so set empty to false
                if (_empty)
                {
                    _empty = false;
                    _cond.wait(lock);
                }
                    // Otherwise set empty to true
                else
                    _empty = true;
                // Get the value from the hold
                auto to_return = std::move(_hold[0]);
                _hold.pop_back();
                // Inform waiting writer and return read value
                _cond.notify_one();
                // Check if poisoned
                if (_strength > 0)
                    throw poison_exception(_strength);
                // Otherwise return value
                return std::move(to_return);
            }

            /*!
             * \brief Extended read operation.
             *
             * \return The value read from the channel.
             */
            T start_read() noexcept(false) override final
            {
                // Lock the mutex
                std::unique_lock<std::mutex> lock(_mut);
                // Check if poisoned
                if (_strength > 0)
                    throw poison_exception(_strength);
                // Check if channel is already reading
                if (_reading)
                    throw std::logic_error("Channel already in extended read");
                // If channel is empty, set empty to false and wait for writer
                if (_empty)
                {
                    _empty = false;
                    _cond.wait(lock);
                }
                    // Otherwise set empty to true
                else
                    _empty = true;
                // Set reading to true
                _reading = true;
                // Check if poisoned
                if (_strength > 0)
                    throw poison_exception(_strength);
                // Return hold value
                return std::move(_hold[0]);
            }

            /*!
             * \brief Extended read completion operation
             */
            void end_read() noexcept(false) override final
            {
                // Lock the mutex
                std::unique_lock<std::mutex> lock(_mut);
                // Check if channel is reading
                if (!_reading)
                    throw std::logic_error("Channel not in extended read");
                // Set empty to true and reading to false
                _hold.pop_back();
                _empty = true;
                _reading = false;
                // Inform waiting writer
                _cond.notify_one();
            }

            /*!
             * \brief Enable the channel with an alt
             *
             * \param[in] a The alt that is being used in the selection.
             *
             * \return True if the channel is ready, false otherwise.
             */
            bool enable(const alt &a) noexcept override final
            {
                // Lock the mutex
                std::unique_lock<std::mutex> lock(_mut);
                // Check if poisoned
                if (_strength > 0)
                    return true;
                // If channel is empty, register alt and return false
                if (_empty)
                {
                    _alt = a;
                    _alting = true;
                    return false;
                }
                    // Otherwise return true
                else
                    return true;
            }

            /*!
             * \brief Disables the channel with an alt.
             *
             * \return True if the channel is ready, false otherwise.
             */
            bool disable() noexcept override final
            {
                // Lock the mutex
                std::unique_lock<std::mutex> lock(_mut);
                _alting = false;
                // Return either if empty, or has been poisoned.
                return !_empty || (_strength > 0);
            }

            /*!
             * \brief Checks if a message is pending on the channel.
             *
             * \return True if the channel is ready, false otherwise.
             */
            bool pending() const noexcept override final
            {
                return !_empty || (_strength > 0);
            }

            /*!
             * \brief Poisons the reading end of the channel.
             *
             * \param[in] strength The strength of the poison to apply to the channel.
             */
            void reader_poison(unsigned int strength) noexcept override final
            {
                // Lock the mutex
                std::unique_lock<std::mutex> lock(_mut);
                // Set strength
                _strength = strength;
                // Notify all waiting processes
                _cond.notify_all();
            }

            /*!
             * \brief Poisons the writer end of the channel
             *
             * \param[in] strength The strength of the poison to apply to the channel.
             */
            void writer_poison(unsigned int strength) noexcept override final
            {
                // Lock the mutex
                std::unique_lock<std::mutex> lock(_mut);
                // Set strength
                _strength = strength;
                // Notify all waiting processes
                _cond.notify_all();
                // If in alt, schedule
                if (_alting)
                    guard::guard_internal::schedule(_alt);
            }

        public:
            /*!
             * \brief Creates a new channel object.
             */
            basic_chan_internal() noexcept { }

            /*!
             * \brief Destroys the channel
             */
            ~basic_chan_internal() noexcept { }
        };

    public:
        /*!
         * \brief Creates a new basic channel
         */
        basic_chan() noexcept
        : chan<T, POISONABLE>(std::shared_ptr<basic_chan_internal>(new basic_chan_internal()))
        {
        }

        /*!
         * \brief Copy constructor.
         *
         * \param[in] other The channel object to copy.
         */
        basic_chan(const basic_chan<T, POISONABLE> &other) = default;

        /*!
         * \brief Move constructor.
         *
         * \param[in] rhs The channel object to copy.
         */
        basic_chan(basic_chan<T, POISONABLE> &&rhs) = default;

        /*!
         * \brief Copy assignment operator.
         *
         * \param[in] other The channel object to copy.
         *
         * \return A copy of the channel object.
         */
        basic_chan<T, POISONABLE>& operator=(const basic_chan<T, POISONABLE> &other) = default;

        /*!
         * \brief Move assignment operator.
         *
         * \param[in] rhs The channel object to copy.
         *
         * \return A copy of the channel object.
         */
        basic_chan<T, POISONABLE>& operator=(basic_chan<T, POISONABLE> &&rhs) = default;

        /*!
         * \brief Destroys the channel object.
         */
        ~basic_chan() { }
    };

    template<typename T, bool POISONABLE = false>
    class fiber_chan : public chan<T, POISONABLE>
    {
    protected:
        class fiber_chan_internal : public chan<T, POISONABLE>::chan_internal
        {
        private:
            mutable boost::fibers::mutex _mut;

            boost::fibers::condition_variable _cond;

            std::vector<T> _hold;

            bool _reading = false;

            bool _empty = true;

            alt _alt;

            bool _alting = false;

            unsigned int _strength = 0;

        protected:
            void write(T value) final
            {
                std::unique_lock<boost::fibers::mutex> lock(_mut);

                if (_strength > 0)
                    throw poison_exception(_strength);
                _hold.push_back(std::move(value));
                if (_empty)
                {
                    _empty = false;
                    if (_alting)
                        guard::guard_internal::schedule(_alt);
                }
                else
                {
                    _empty = true;
                    _cond.notify_one();
                }
                _cond.wait(lock);
                if (_strength > 0)
                    throw poison_exception(_strength);
            }

            T read() final
            {
                std::unique_lock<boost::fibers::mutex> lock(_mut);
                if (_strength > 0)
                    throw poison_exception(_strength);
                if (_empty)
                {
                    _empty = false;
                    _cond.wait(lock);
                }
                else
                    _empty = true;
                auto to_return = std::move(_hold[0]);
                _hold.pop_back();
                _cond.notify_one();
                if (_strength > 0)
                    throw poison_exception(_strength);
                return std::move(to_return);
            }

            T start_read() final
            {
                std::unique_lock<boost::fibers::mutex> lock(_mut);
                if (_strength > 0)
                    throw poison_exception(_strength);
                if (_reading)
                    throw std::logic_error("Channel already in extended read");
                if (_empty)
                {
                    _empty = false;
                    _cond.wait(lock);
                }
                else
                    _empty =true;
                _reading = true;
                if (_strength > 0)
                    throw poison_exception(_strength);
                return std::move(_hold[0]);
            }

            void end_read() final
            {
                std::unique_lock<boost::fibers::mutex> lock(_mut);
                if (!_reading)
                    throw std::logic_error("Channel not in extended read");
                _hold.pop_back();
                _empty = true;
                _reading = false;
                _cond.notify_one();
            }

            bool enable(const alt &a) noexcept final
            {
                std::unique_lock<boost::fibers::mutex> lock(_mut);
                if (_strength > 0)
                    return true;
                if (_empty)
                {
                    _alt = a;
                    _alting = true;
                    return false;
                }
                else
                    return true;
            }

            bool disable() noexcept final
            {
                std::unique_lock<boost::fibers::mutex> lock(_mut);
                _alting = false;
                return !_empty || (_strength > 0);
            }

            bool pending() const noexcept final
            {
                return !_empty || (_strength > 0);
            }

            void reader_poison(unsigned int strength) noexcept final
            {
                std::unique_lock<boost::fibers::mutex> lock(_mut);
                _strength = strength;
                _cond.notify_all();
            }

            void writer_poison(unsigned int strength) noexcept final
            {
                std::unique_lock<boost::fibers::mutex> lock(_mut);
                _strength = strength;
                _cond.notify_all();
                if (_alting)
                    guard::guard_internal::schedule(_alt);
            }

        public:
            fiber_chan_internal() noexcept { }

            ~fiber_chan_internal() noexcept { }
        };

    public:
        fiber_chan()
        : chan<T, POISONABLE>(std::shared_ptr<fiber_chan_internal>(new fiber_chan_internal()))
        {
        }

        fiber_chan(const fiber_chan<T, POISONABLE>&) = default;

        fiber_chan(fiber_chan<T, POISONABLE>&&) = default;

        fiber_chan& operator=(const fiber_chan<T, POISONABLE>&) = default;

        fiber_chan& operator=(fiber_chan<T, POISONABLE>&&) = default;

        ~fiber_chan() { }
    };

    /*! \class busy_chan
     * \brief A busy (spinning) implementation of a channel.
     *
     * \tparam T the type that the channel operates on.
     * \tparam POISONABLE Flag to indicate if the channel can be poisoned.
     *
     * \author Kevin Chalmers
     *
     * \date 17/10/2016
     */
    template<typename T, bool POISONABLE = false>
    class busy_chan : public chan<T, POISONABLE>
    {
    protected:
        /*! \class busy_chan_internal
         * \brief Internal representation of a busy channel.
         *
         * \author Kevin Chalmers
         *
         * \date 17/10/2016
         */
        class busy_chan_internal : public chan<T, POISONABLE>::chan_internal
        {
        private:

            std::atomic<T> _hold; //!< Current value on the channel.

            std::atomic<bool> _reading; //!< Flag used to control and indicate state of reading process.

            std::atomic<bool> _writing; //!< Flag used to control and indicate state of writing process.

            alt _alt; //!< Alt used when channel is in a selection operation.

            std::atomic<bool> _alting; //!< Flag used to indicate whether the channel is being used in a selection operation.

            std::atomic<unsigned int> _strength; //!< Strength of poison on channel.

        protected:

            /*!
             * \brief Performs a write operation on the channel.
             *
             * \param[in] value The value to write to the channel.
             */
            void write(T value) noexcept(false) override final
            {
                // Store value
                _hold.store(value, std::memory_order_relaxed);
                // Set writing to true - will inform reading end if spinning.
                _writing.store(true, std::memory_order_release);
                // Now check if an alt has occurred at some point prior to now.
                if (_alting.load(std::memory_order_acquire))
                    guard::guard_internal::schedule(_alt);
                // Now spin waiting for reader to grab the date
                while (!_reading.load(std::memory_order_acquire));
                // At this point, we can check if we have been poisoned.  Poisoning will leave reading as true.  Therefore,
                // if poisoning happened, it happened before now.
                if (_strength.load(std::memory_order_relaxed) > 0)
                    throw poison_exception(_strength.load(std::memory_order_relaxed));
                // At this point, we know that the reader has the value.  It will at some point wait for us to signal
                // we are finished (it may already be waiting for that).  Set writing to false to signal.
                _writing.store(false, std::memory_order_release);
                // Now we need to wait until the reader has seen that we are finished.
                while (_reading.load(std::memory_order_acquire));
                // We have now completed.  The state visible to each thread should be consistent enough to continue without
                // a hazard.
            }

            /*!
             * \brief Performs a read operation on the channel.
             *
             * \return The value read from the channel.
             */
            T read() noexcept(false) override final
            {
                // Spin for the writer.  Cannot progress until writer has started.
                while (!_writing.load(std::memory_order_acquire));
                // At this point, we can check if we have been poisoned.  Poisoning will leave writing as true.  Therefore,
                // if poisoning happened, it happened before now.
                if (_strength.load(std::memory_order_relaxed) > 0)
                    throw poison_exception(_strength.load(std::memory_order_relaxed));
                // At this point, we know the writer has at least stored the value in the hold.  So let's grab it.
                auto to_return = _hold.load(std::memory_order_relaxed);
                // Now we need to inform the writer that we have grabbed the value.  It will at some point wait for us to
                // signal we have done so (it may already be waiting for that).  Set reading to true to signal.
                _reading.store(true, std::memory_order_release);
                // Now we need to wait for the writer to signal that it has seen we have grabbed the value, and signal
                // that it has effectively completed.
                while (_writing.load(std::memory_order_acquire));
                // The writer will at some point be waiting for us to signal that we have also completed.
                _reading.store(false, std::memory_order_release);
                // We have now completed.  The state visible to each thread should be consistent enough to continue without
                // a hazard.
                return to_return;
            }

            /*!
             * \brief Starts an extended read operation.
             *
             * \return The value read from the channel.
             */
            T start_read() noexcept(false) override final
            {
                // Sanity check - make sure we are no reading.
                if (_reading.load(std::memory_order_acquire))
                    throw std::logic_error("Channel already in extended read");
                // Spin for the writer.  Cannot progress until writer has started.
                while (!_writing.load(std::memory_order_acquire));
                // At this point, we can check if we have been poisoned.  Poisoning will leave writing as true.  Therefore,
                // if poisoning happened, it happened before now.
                if (_strength.load(std::memory_order_relaxed) > 0)
                    throw poison_exception(_strength.load(std::memory_order_relaxed));
                // At this point, we know the writer has at least stored the value in the hold.  So let's grab it.
                auto to_return = _hold.load(std::memory_order_relaxed);
                // Now we need to inform the writer that we have grabbed the value.  It will at some point wait for us to
                // signal we have done so (it may already be waiting for that).  Set reading to true to signal.
                _reading.store(true, std::memory_order_release);
                // Now we need to wait for the writer to signal that it has seen we have grabbed the value, and signal
                // that it has effectively completed.
                while (_writing.load(std::memory_order_acquire));
                // In a normal read, we would now inform the writer.  However, we are extended.  Just return the value.
                return to_return;
            }

            /*!
             * \brief Completes extended read operation.
             */
            void end_read() noexcept(false) override final
            {
                // Ensure we are in an extended read.
                if (!_reading.load(std::memory_order_acquire))
                    throw std::logic_error("Channel not in extended read");
                // At this point, all we really need to do is set reading to false.  The writer is currently spinning
                // waiting for this value (or at least will be if not already).
                _reading.store(false, std::memory_order_release);
            }

            /*!
             * \brief Enable the channel with an alt
             *
             * \param[in] a The alt that is being used in the selection.
             *
             * \return True if the channel is ready, false otherwise.
             */
            bool enable(const alt &a) noexcept override final
            {
                // Set alt and alting
                _alt = a;
                _alting.store(true, std::memory_order_release);
                // Check if writing
                auto local_writing = _writing.load(std::memory_order_acquire);
                // At this point we know one of the following:
                // 1. local_writing is true, and the write has committed before seeing alting
                // 2. local_writing is true, and the channel is poisoned
                // 3. local_writing is true, and the write saw alting was true before committing
                // 4. local_writing is false, and the write has not committed
                // Deal with 2 first
                // Check if poisoned
                if (_strength.load(std::memory_order_relaxed) > 0)
                    return true;
                // 1 & 3 mean we are in a committed write.
                // 4 will be dealt with by memory ordering.  The write will see alting as true and act accordingly.
                // In both cases, local_writing contains the value to return.
                return local_writing;
            }

            /*!
             * \brief Disables the channel with an alt.
             *
             * \return True if the channel is ready, false otherwise.
             */
            bool disable() noexcept override final
            {
                // First set alting to false
                _alting.store(false, std::memory_order_release);
                // Now get the value of writing
                return _writing.load(std::memory_order_acquire);
                // At this point we know one of the following:
                // 1. The write has not committed.
                // 2. The write has committed, and previously saw alting as true.
                // 3. The write has committed, but never saw alting as true.
                // 1 will return false (the value of local_writing)
                // 2 & 3 will return true.  The write has committed, so is ready.  This is the value of local_writing.
                // In both cases we can return local_writing.
            }

            /*!
             * \brief Checks if a message is pending on the channel.
             *
             * \return True if the channel is ready, false otherwise.
             */
            bool pending() const noexcept override final
            {
                // Just check if we are in a writing state.
                return _writing.load(std::memory_order_acquire);
            }

            /*!
             * \brief Poisons the reading end of the channel.
             *
             * \param[in] strength The strength of the poison to apply to the channel.
             */
            void reader_poison(unsigned int strength) noexcept override final
            {
                // First set the poison value.
                _strength.store(strength, std::memory_order_relaxed);
                // And now set reading to true.  Will ensure any writing process will see the poison.
                _reading.store(true, std::memory_order_release);
            }

            /*!
             * \brief Poisons the writer end of the channel
             *
             * \param[in] strength The strength of the poison to apply to the channel.
             */
            void writer_poison(unsigned int strength) noexcept override final
            {
                // First set the poison value.
                _strength.store(strength, std::memory_order_relaxed);
                // And now set writing to true.  Will ensure any reading process will see the poison.
                _writing.store(true);
                // Now we need to check alting.  We know one of the following has happened.
                // 1. An enabling process has seen writing as true, and therefore sees the poison.
                // 2. An enabling process has seen writing as false, but we see alting as true.  Therefore we can notify
                // the enabling process.
                // The enabling process cannot see writing as false when alting is false.  In this case, we know no
                // enabling is occurring.
                // Therefore, if alting, schedule.
                if (_alting.load(std::memory_order_acquire))
                    guard::guard_internal::schedule(_alt);
            }

        public:
            /*!
             * \brief Creates a new channel object.
             */
            busy_chan_internal() noexcept
            : _reading(false), _writing(false), _alting(false), _strength(0)
            {
            }

            /*!
             * \brief Destroys the channel
             */
            ~busy_chan_internal() noexcept { }
        };
    public:
        /*!
         * \brief Creates a new busy channel
         */
        busy_chan() noexcept
        : chan<T, POISONABLE>(std::shared_ptr<busy_chan_internal>(new busy_chan_internal()))
        {
        }

        /*!
         * \brief Copy constructor.
         *
         * \param[in] other The channel object to copy.
         */
        busy_chan(const busy_chan<T, POISONABLE> &other) = default;

        /*!
         * \brief Move constructor.
         *
         * \param[in] rhs The channel object to copy.
         */
        busy_chan(busy_chan<T, POISONABLE> &&rhs) = default;

        /*!
         * \brief Copy assignment operator.
         *
         * \param[in] other The channel object to copy.
         *
         * \return A copy of the channel object.
         */
        busy_chan<T, POISONABLE>& operator=(const busy_chan<T, POISONABLE> &other) = default;

        /*!
         * \brief Move assignment operator.
         *
         * \param[in] rhs The channel object to copy.
         *
         * \return A copy of the channel object.
         */
        busy_chan<T, POISONABLE>& operator=(busy_chan<T, POISONABLE> &&rhs) = default;

        /*!
         * \brief Destroys the channel object.
         */
        ~busy_chan() { }
    };

    /*! \class bufferd_chan
     * \brief Buffered implementation of a channel.
     *
     * \tparam T The type that the channel operates on.
     * \tparam POISONABLE Flag to indicate if the channel can be poisoned.
     *
     * \author Kevin Chalmers
     *
     * \date 28/04/2016
     */
    template<typename T, bool POISONABLE = false>
    class buffered_chan : public chan<T, POISONABLE>
    {
        // Friend declarations.
        friend class one2one_chan<T, POISONABLE>;
        friend class one2any_chan<T, POISONABLE>;
        friend class any2one_chan<T, POISONABLE>;
        friend class any2any_chan<T, POISONABLE>;
    protected:
        /*! \class buffered_chan_internal
         * \brief Internal implementation of a buffered channel.
         *
         * \author Kevin Chalmers
         *
         * \date 28/04/2016
         */
        class buffered_chan_internal : public chan<T, POISONABLE>::chan_internal
        {
        private:

            chan_data_store<T> _buffer; //<! The internal buffer used to store messages.

            mutable std::mutex _mut; //<! Lock used to control access to the channel.

            std::condition_variable _cond; //<! Condition variable used to wait for events.

            bool _reading = false; //<! Flag to indicate whether the channel is in an extended read operation.

            alt _alt; //<! Alt used when channel is in a selection operation.

            bool _alting = false; //<! Flag used to indicate whether the channel is in a selection operation.

            unsigned int _strength = 0; //<! Strength of poison on the channel.

        protected:
            /*!
             * \brief Performs a write operation on the channel.
             *
             * \param[in] value The value to write to the channel.
             */
            void write(T value) noexcept(false) override final
            {
                // Lock the mutex
                std::unique_lock<std::mutex> lock(_mut);
                // Check if poisoned
                if (_strength > 0)
                    throw poison_exception(_strength);
                // Put the value in the buffer
                _buffer.put(std::move(value));
                // If channel is in select then inform alt, otherwise inform reader
                if (_alting)
                    guard::guard_internal::schedule(_alt);
                else
                    _cond.notify_one();
                // Check if buffer is full and wait if it is
                if (_buffer.get_state() == DATA_STORE_STATE::FULL)
                    _cond.wait(lock);
                // Check if poisoned
                if (_strength > 0)
                    throw poison_exception(_strength);
            }

            /*!
             * \brief Performs a read operation on the channel.
             *
             * \return The value read from the channel.
             */
            T read() noexcept(false) override final
            {
                // Lock the mutex
                std::unique_lock<std::mutex> lock(_mut);
                // Check if poisoned
                if (_strength > 0)
                    throw poison_exception(_strength);
                // Check if buffer is empty, and if so wait until a write occurs.
                if (_buffer.get_state() == DATA_STORE_STATE::EMPTY)
                    _cond.wait(lock);
                // Inform any waiting writers
                _cond.notify_one();
                // Check if poisoned
                if (_strength > 0)
                    throw poison_exception(_strength);
                // Return value in the buffer.
                return std::move(_buffer.get());
            }

            /*!
             * \brief Starts an extended read operation.
             *
             * \return The value read from the channel.
             */
            T start_read() noexcept(false) override final
            {
                // Lock the mutex
                std::unique_lock<std::mutex> lock(_mut);
                // Check if poisoned
                if (_strength > 0)
                    throw poison_exception(_strength);
                // Ensure that we are not already reading.
                if (_reading)
                    throw std::logic_error("Channel already in extended read");
                // If buffer is empty then we wait
                if (_buffer.get_state() == DATA_STORE_STATE::EMPTY)
                    _cond.wait(lock);
                // Set reading flag to true and return value in the buffer.
                _reading = true;
                // Check if poisoned
                if (_strength > 0)
                    throw poison_exception(_strength);
                return std::move(_buffer.get());
            }

            /*!
             * \brief Ends an extended read operation.
             */
            void end_read() noexcept(false) override final
            {
                // Lock the mutex
                std::unique_lock<std::mutex> lock(_mut);
                // Check that channel is in a reading state
                if (_reading)
                    throw std::logic_error("Channel not in extended read");
                // Inform any waiting writer
                _cond.notify_one();
                // Set reading flag to false
                _reading = false;
            }

            /*!
             * \brief Enables the channel during an alt operation.
             *
             * \param[in] a The alt being used in the selection.
             *
             * \return True if the channel is ready, false otherwise.
             */
            bool enable(const alt &a) noexcept override final
            {
                // Lock the mutex
                std::unique_lock<std::mutex> lock(_mut);
                // Check if poisoned
                if (_strength > 0)
                    return true;
                // If buffer is empty, then set alt and return false
                if (_buffer.get_state() == DATA_STORE_STATE::EMPTY)
                {
                    _alting = true;
                    _alt = a;
                    return false;
                }
                // Otherwise return true
                else
                    return true;
            }

            /*!
             * \brief Disables the channel during an alt operation.
             *
             * \return True if the channel is ready, false otherwise.
             */
            bool disable() noexcept override final
            {
                // Lock the mutex
                std::unique_lock<std::mutex> lock(_mut);
                // Set alting to false
                _alting = false;
                // Return whether the buffer has a value within it
                return _buffer.get_state() != DATA_STORE_STATE::EMPTY || _strength > 0;
            }

            /*!
             * \brief Checks if a message is pending on the channel.
             *
             * \return True if a value is on the channel, false otherwise.
             */
            bool pending() const noexcept override final
            {
                // Lock the mutex
                std::unique_lock<std::mutex> lock(_mut);
                // Return whether the buffer has a value within it.
                return _buffer.get_state() != DATA_STORE_STATE::EMPTY || _strength > 0;
            }

            /*!
             * \brief Poisons the reading end of the channel.
             *
             * \param[in] strength The strength of poison to apply to the channel.
             */
            void reader_poison(unsigned int strength) noexcept override final
            {
                // Lock the mutex
                std::unique_lock<std::mutex> lock(_mut);
                // Set strength
                _strength = strength;
                // Notify all waiting processes
                _cond.notify_all();
            }

            /*!
             * \brief Poisons the writing end of the channel.
             *
             * \param[in] strength The strength of the poison to apply to the channel.
             */
            void writer_poison(unsigned int strength) noexcept override final
            {
                // Lock the mutex
                std::unique_lock<std::mutex> lock(_mut);
                // Set strength
                _strength = strength;
                // Notify all waiting processes
                _cond.notify_all();
                // If in alt, schedule
                if (_alting)
                    guard::guard_internal::schedule(_alt);
            }
        public:
            /*!
             * \brief Creates a new internal buffered channel.
             *
             * \param[in] buffer Buffer used within the underlying channel
             */
            buffered_chan_internal(chan_data_store<T> &buffer) noexcept
            : _buffer(buffer)
            {
            }

            /*!
             * \brief Destroys the internal buffered channel.
             */
            ~buffered_chan_internal() noexcept { }
        };

    public:

        /*!
         * \brief Creates a new buffered channel.
         *
         * \param[in] buffer The buffer to use in the channel.
         */
        buffered_chan(chan_data_store<T> &buffer) noexcept
        : chan<T, POISONABLE>(std::shared_ptr<buffered_chan_internal>(new buffered_chan_internal(buffer)))
        {
        }

        /*!
         * \brief Copy constructor.
         *
         * \param[in] other The buffered channel to copy.
         */
        buffered_chan(const buffered_chan<T, POISONABLE> &other) noexcept = default;

        /*!
         * \brief Move constructor.
         *
         * \param[in] rhs The buffered channel to copy.
         */
        buffered_chan(buffered_chan<T, POISONABLE> &&rhs) noexcept = default;

        /*!
         * \brief Copy assignment operator.
         *
         * \param[in] other The buffered channel to copy.
         *
         * \return A copy of the buffered channel.
         */
        buffered_chan<T, POISONABLE>& operator=(const buffered_chan<T, POISONABLE> &other) noexcept = default;

        /*!
         * \brief Move assignment operator.
         *
         * \param[in] rhs The buffered channel to copy.
         *
         * \return A copy of the buffered channel.
         */
        buffered_chan<T, POISONABLE>& operator=(buffered_chan<T, POISONABLE> &&rhs) noexcept = default;

        /*!
         * \brief Destroys the channel.
         */
        ~buffered_chan() { }
    };

    template<typename T, bool POISONABLE = false>
    class fiber_buffered_chan : public chan<T, POISONABLE>
    {
        friend class fiber_one2one_chan<T, POISONABLE>;
        friend class fiber_one2any_chan<T, POISONABLE>;
        friend class fiber_any2one_chan<T, POISONABLE>;
        friend class fiber_any2any_chan<T, POISONABLE>;
    protected:
        class fiber_buffered_chan_internal : public chan<T, POISONABLE>::internal
        {
        private:
            chan_data_store<T> _buffer;

            mutable boost::fibers::mutex _mut;

            boost::fibers::condition_variable _cond;

            bool _reading = false;

            alt _alt;

            bool _alting = false;

            unsigned int _strength = 0;

        protected:
            void write(T value) final
            {
                std::unique_lock<boost::fibers::mutex> lock(_mut);
                if (_strength > 0)
                    throw poison_exception(_strength);
                _buffer.put(std::move(value));
                if (_alting)
                    guard::guard_internal::schedule(_alt);
                else
                    _cond.notify_one();
                if (_buffer.get_state() == DATA_STORE_STATE::FULL)
                    _cond.wait(lock);
                if (_strength > 0)
                    throw poison_exception(_strength);
            }

            T read() final
            {
                std::unique_lock<boost::fibers::mutex> lock(_mut);
                if (_strength > 0)
                    throw poison_exception(_strength);
                if (_buffer.get_state() == DATA_STORE_STATE::EMPTY)
                    _cond.wait(lock);
                _cond.notify_one();
                if (_strength > 0)
                    throw poison_exception(_strength);
                return std::move(_buffer.get());
            }

            T start_read() final
            {
                std::unique_lock<boost::fibers::mutex> lock(_mut);
                if (_strength > 0)
                    throw poison_exception(_strength);
                if (_reading)
                    throw std::logic_error("Channel already in extended read state");
                if (_buffer.get_state() == DATA_STORE_STATE::EMPTY)
                    _cond.wait(lock);
                _reading = true;
                if (_strength > 0)
                    throw poison_exception(_strength);
                return std::move(_buffer.get());
            }

            void end_read() final
            {
                std::unique_lock<boost::fibers::mutex> lock(_mut);
                if (_reading)
                    throw std::logic_error("Channel not in extended read");
                _cond.notify_one();
                _reading = false;
            }

            bool enable(const alt &a) noexcept final
            {
                std::unique_lock<boost::fibers::mutex> lock(_mut);
                if (_strength > 0)
                    return true;
                if (_buffer.get_state() == DATA_STORE_STATE::EMPTY)
                {
                    _alting = true;
                    _alt = a;
                    return false;
                }
                else
                    return true;
            }

            bool disable() noexcept final
            {
                std::unique_lock<boost::fibers::mutex> lock(_mut);
                _alting = false;
                return _buffer.get_state() != DATA_STORE_STATE::EMPTY || _strength > 0;
            }

            bool pending() const noexcept final
            {
                std::unique_lock<boost::fibers::mutex> lock(_mut);
                return _buffer.get_state() != DATA_STORE_STATE::EMPTY || _strength > 0;
            }

            void reader_poison(unsigned int strength) noexcept final
            {
                std::unique_lock<boost::fibers::mutex> lock(_mut);
                _strength = strength;
                _cond.notify_all();
            }

            void writer_poison(unsigned int strength) noexcept final
            {
                std::unique_lock<boost::fibers::mutex> lock(_mut);
                _strength = strength;
                _cond.notify_all();
                if (_alting)
                    guard::guard_internal::schedule(_alt);
            }

        public:
            fiber_buffered_chan_internal(chan_data_store<T> &buffer)
            : _buffer(buffer)
            {
            }

            ~fiber_buffered_chan_internal() noexcept { }
        };

    public:
        fiber_buffered_chan(chan_data_store<T> &buffer)
        : chan<T, POISONABLE>(std::shared_ptr<fiber_buffered_chan_internal>(new fiber_buffered_chan_internal(buffer)))
        {
        }

        fiber_buffered_chan(const fiber_buffered_chan<T, POISONABLE>&) = default;

        fiber_buffered_chan(fiber_buffered_chan<T, POISONABLE>&&) = default;

        fiber_buffered_chan<T, POISONABLE>& operator=(const fiber_buffered_chan<T, POISONABLE>&) = default;

        fiber_buffered_chan<T, POISONABLE>& operator=(fiber_buffered_chan<T, POISONABLE>&&) = default;

        ~fiber_buffered_chan() { }
    };

    /*! \class busy_bufferd_chan
     * \brief Buffered implementation of a busy channel.
     *
     * \tparam T The type that the channel operates on.
     * \tparam POISONABLE Flag to indicate if the channel can be poisoned.
     *
     * \author Kevin Chalmers
     *
     * \date 19/10/2016
     */
    template<typename T, bool POISONABLE = false>
    class busy_buffered_chan : public chan<T, POISONABLE>
    {
        // Friend declarations.
        friend class busy_one2one_chan<T, POISONABLE>;
        friend class busy_one2any_chan<T, POISONABLE>;
        friend class busy_any2one_chan<T, POISONABLE>;
        friend class busy_any2any_chan<T, POISONABLE>;
    protected:
        /*! \class buffered_chan_internal
         *
         */
        class busy_buffered_chan_internal : public chan<T, POISONABLE>::chan_internal
        {
        private:

            std::atomic<chan_data_store<T>> _buffer; //<! The internal buffer used to store messages.

            alt _alt; //<! Alt used when channel is in a selection operation.

            std::atomic<bool> _alting; //<! Flag used to indicate whether the channel is in a selection operation.

            bool _reading = false; //<! Flag used to indicate whether the channel is in an extended read state.

            std::atomic<unsigned int> _strength; //<! Strength of poison on the channel.

        protected:
            /*!
             * \brief Performs a write operation on the channel.
             *
             * \param[in] value The value to write to the channel.
             */
            void write(T value) noexcept(false) override final
            {
                // At this point we know one of two things
                // 1. The buffer is not full; or
                // 2. The channel is poisoned.
                // The former is blocking, so check the latter first
                if (_strength.load() > 0)
                    throw poison_exception(_strength.load());
                // OK, we now know we aren't poisoned.  So write to the buffer.
                _buffer.load().put(value);
                // If channel is in select then inform alt, otherwise inform reader
                if (_alting.load())
                    guard::guard_internal::schedule(_alt);
                // Spin until the buffer is full
                while (_buffer.load().get_state() == DATA_STORE_STATE::FULL);
                // Final check if poisoned
                if (_strength.load() > 0)
                    throw poison_exception(_strength.load());
            }

            /*!
             * \brief Performs a read operation on the channel.
             *
             * \return The value read from the channel.
             */
            T read() noexcept(false) override final
            {
                // First check if poisoned
                if (_strength.load() > 0)
                    throw poison_exception(_strength.load());
                // Now we need to wait until the buffer is not empty
                while (_buffer.load().get_state() == DATA_STORE_STATE::EMPTY);
                // Check if poisoned again.  This maybe what woke us
                if (_strength.load() > 0)
                    throw poison_exception(_strength.load());
                // Return value in the buffer.
                return std::move(_buffer.load().get());
            }

            /*!
             * \brief Starts an extended read operation.
             *
             * \return The value read from the channel.
             */
            T start_read() noexcept(false) override final
            {
                // First ensure we are not already reading
                if (_reading)
                    throw std::logic_error("Channel already in extended read");
                // First check if poisoned
                if (_strength.load() > 0)
                    throw poison_exception(_strength.load());
                // Now we need to wait until the buffer is not empty
                while (_buffer.load().get_state() == DATA_STORE_STATE::EMPTY);
                // Set reading to true
                _reading = true;
                // Check if poisoned again.  This maybe what woke us
                if (_strength.load() > 0)
                    throw poison_exception(_strength.load());
                // Return value peeked in the buffer.
                return std::move(_buffer.load().peek());
            }

            /*!
             * \brief Ends an extended read operation.
             */
            void end_read() noexcept(false) override final
            {
                // Check that channel is in a reading state
                if (!_reading)
                    throw std::logic_error("Channel not in extended read");
                // Set reading flag to false
                _reading = false;
                // And actually remove value from buffer.
                _buffer.load().get();
            }

            /*!
             * \brief Enables the channel during an alt operation.
             *
             * \param[in] a The alt being used in the selection.
             *
             * \return True if the channel is ready, false otherwise.
             */
            bool enable(const alt &a) noexcept override final
            {
                // First check if poisoned
                if (_strength.load() > 0)
                    return true;
                // Store alting as true.  We have two cases.
                // 1. writer got here first.  In that case, it will have already stored its value and our check later will be true
                // 2. we got here first.  In that case, the writer will see alting and notify us if needs be.
                _alting.store(true);
                // If buffer is empty, then set alt and return false
                if (_buffer.load().get_state() == DATA_STORE_STATE::EMPTY)
                {
                    _alt = a;
                    return false;
                }
                // Otherwise return true
                else
                    return true;
            }

            /*!
             * \brief Disables the channel during an alt operation.
             *
             * \return True if the channel is ready, false otherwise.
             */
            bool disable() noexcept override final
            {
                // Set alting to false
                _alting.store(false);
                // Return whether the buffer has a value within it
                return _buffer.load().get_state() != DATA_STORE_STATE::EMPTY || _strength.load() > 0;
            }

            /*!
             * \brief Checks if a message is pending on the channel.
             *
             * \return True if a value is on the channel, false otherwise.
             */
            bool pending() const noexcept override final
            {
                // Return whether the buffer has a value within it.
                return _buffer.load().get_state() != DATA_STORE_STATE::EMPTY || _strength.load() > 0;
            }

            /*!
             * \brief Poisons the reading end of the channel.
             *
             * \param[in] strength The strength of poison to apply to the channel.
             */
            void reader_poison(unsigned int strength) noexcept override final
            {
                // Set strength
                _strength.store(strength);
                // We now need to ensure that the writer will always go.
                // Set buffer state to NONEMPTYFULL
                if (_buffer.load().get_state() == DATA_STORE_STATE::FULL)
                    _buffer.load().get();
            }

            /*!
             * \brief Poisons the writing end of the channel.
             *
             * \param[in] strength The strength of the poison to apply to the channel.
             */
            void writer_poison(unsigned int strength) noexcept override final
            {
                // Set strength
                _strength.store(strength);
                // TODO - how do we ensure that the buffer is not empty, and hence the reader goes.  I think I need set state.
                // If in alt, schedule
                if (_alting.load())
                    guard::guard_internal::schedule(_alt);
            }
        public:
            /*!
             * \brief Creates a new internal buffered channel.
             *
             * \param[in] buffer Buffer used within the underlying channel
             */
            busy_buffered_chan_internal(chan_data_store<T> &buffer) noexcept
            : _buffer(buffer), _alting(false), _strength(0)
            {
            }

            /*!
             * \brief Destroys the internal buffered channel.
             */
            ~busy_buffered_chan_internal() noexcept { }
        };

    public:

        /*!
         * \brief Creates a new buffered channel.
         *
         * \param[in] buffer The buffer to use in the channel.
         */
        busy_buffered_chan(chan_data_store<T> &buffer) noexcept
        : chan<T, POISONABLE>(std::shared_ptr<busy_buffered_chan_internal>(new busy_buffered_chan_internal(buffer)))
        {
        }

        /*!
         * \brief Copy constructor.
         *
         * \param[in] other The buffered channel to copy.
         */
        busy_buffered_chan(const busy_buffered_chan<T, POISONABLE> &other) noexcept = default;

        /*!
         * \brief Move constructor.
         *
         * \param[in] rhs The buffered channel to copy.
         */
        busy_buffered_chan(busy_buffered_chan<T, POISONABLE> &&rhs) noexcept = default;

        /*!
         * \brief Copy assignment operator.
         *
         * \param[in] other The buffered channel to copy.
         *
         * \return A copy of the buffered channel.
         */
        busy_buffered_chan<T, POISONABLE>& operator=(const busy_buffered_chan<T, POISONABLE> &other) noexcept = default;

        /*!
         * \brief Move assignment operator.
         *
         * \param[in] rhs The buffered channel to copy.
         *
         * \return A copy of the buffered channel.
         */
        busy_buffered_chan<T, POISONABLE>& operator=(busy_buffered_chan<T, POISONABLE> &&rhs) noexcept = default;

        /*!
         * \brief Destroys the channel.
         */
        ~busy_buffered_chan() { }
    };

    // TODO: one2one symmetric channel.

    /*! \class one2one_chan
     * \brief A channel with a guarded input end and a non-shared output end.
     *
     * \tparam T Type that the channel operates on.
     * \tparam POISONABLE Flag to indicate whether this channel can be poisoned.
     *
     * \author Kevin Chalmers
     *
     * \date 17/04/2016
     */
    template<typename T, bool POISONABLE = false>
    class one2one_chan
    {
    private:
        // Type declarations used by channel.
        using INPUT = alting_chan_in<T, POISONABLE>;
        using INPUT_IMPL = typename INPUT::alting_chan_in_internal;
        using OUTPUT = chan_out<T, POISONABLE>;
        using OUTPUT_IMPL = typename OUTPUT::chan_out_internal;

        chan<T, POISONABLE> _chan; //<! Internal channel implementation.

        INPUT _in; //<! The input end of the channel.

        OUTPUT _out; //<! The output end of the channel.

    public:
        /*!
         * \brief Creates a new one2one channel
         *
         * \param[in] immunity The poison immunity level that the channel has.
         */
        one2one_chan(unsigned int immunity = 0) noexcept
        : _chan(basic_chan<T, POISONABLE>()),
          _in(std::shared_ptr<INPUT_IMPL>(new INPUT_IMPL(_chan, immunity))),
          _out(std::shared_ptr<OUTPUT_IMPL>(new OUTPUT_IMPL(_chan, immunity)))
        {
        }

        /*!
         * \brief Creates a new buffered one2one channel
         *
         * \param[in] buffer The buffer to use with the channel.
         * \param[in] immunity The poison immunity level that the channel has.
         */
        one2one_chan(chan_data_store<T> &buffer, unsigned int immunity = 0) noexcept
        : _chan(buffered_chan<T, POISONABLE>(buffer)),
          _in(std::shared_ptr<INPUT_IMPL>(new INPUT_IMPL(_chan, immunity))),
          _out(std::shared_ptr<OUTPUT_IMPL>(new OUTPUT_IMPL(_chan, immunity)))
        {
        }

        /*!
         * \brief Gets the input end of the one2one channel.
         *
         * \return The input end of the channel.
         */
        alting_chan_in<T, POISONABLE> in() const noexcept { return _in; }

        /*!
         * \brief Gets the output end of the one2one channel.
         *
         * \return The output end of the channel.
         */
        chan_out<T, POISONABLE> out() const noexcept { return _out; }

        /*!
         * \brief Conversion operator.  Implicitly gets input end.
         *
         * \return The input end of the channel.
         */
        operator alting_chan_in<T, POISONABLE>() const noexcept { return _in; }

        /*!
         * \brief Conversion operator.  Implicitly gets output end.
         *
         * \return The output end of the channel.
         */
        operator chan_out<T, POISONABLE>() const noexcept { return _out; }

        /*!
         * \brief Performs a read on the channel.
         *
         * \return Value read from the channel.
         */
        T operator()() const noexcept
        {
            return _in.read();
        }

        /*!
         * \brief Performs a write on the channel.
         *
         * \param[in] value Value to write to the channel.
         */
        void operator()(T value) const noexcept
        {
            _out.write(value);
        }
    };

    template<typename T, bool POISONABLE = false>
    class fiber_one2one_chan
    {
    private:
        using INPUT = alting_chan_in<T, POISONABLE>;
        using INPUT_IMPL = typename INPUT::alting_chan_in_internal;
        using OUTPUT = chan_out<T, POISONABLE>;
        using OUTPUT_IMPL = typename OUTPUT::chan_out_internal;

        chan<T, POISONABLE> _chan;

        INPUT _in;

        OUTPUT _out;

    public:
        fiber_one2one_chan(unsigned int immunity = 0)
        : _chan(fiber_chan<T, POISONABLE>()),
          _in(std::shared_ptr<INPUT_IMPL>(new INPUT_IMPL(_chan, immunity))),
          _out(std::shared_ptr<OUTPUT_IMPL>(new OUTPUT_IMPL(_chan, immunity)))
        {
        }

        fiber_one2one_chan(chan_data_store<T> &buffer, unsigned int immunity = 0)
        : _chan(fiber_buffered_chan<T, POISONABLE>()),
          _in(std::shared_ptr<INPUT_IMPL>(new INPUT_IMPL(_chan, immunity))),
          _out(std::shared_ptr<OUTPUT_IMPL>(new OUTPUT_IMPL(_chan, immunity)))
        {
        }

        alting_chan_in<T, POISONABLE> in() const noexcept { return _in; };

        chan_out<T, POISONABLE> out() const noexcept { return _out; };

        operator alting_chan_in<T, POISONABLE>() const noexcept { return _in; };

        operator chan_out<T, POISONABLE>() const noexcept { return _out; };

        T operator()() const noexcept
        {
            return _in.read();
        }

        void operator()(T value) const noexcept
        {
            _out.write(value);
        }
    };

    /*!
     * \class busy_one2one_chan
     * \brief A one2one channel that uses busy semantics.
     *
     * \tparam T The type that the channel operates on.
     * \tparam POISONABLE Flag to indicate if the channel can be poisoned.
     *
     * \author Kevin Chalmers
     *
     * \date 18/10/2016
     */
    template<typename T, bool POISONABLE = false>
    class busy_one2one_chan
    {
    private:
        // Type declarations used by channel.
        using INPUT = alting_chan_in<T, POISONABLE>;
        using INPUT_IMPL = typename INPUT::alting_chan_in_internal;
        using OUTPUT = chan_out<T, POISONABLE>;
        using OUTPUT_IMPL = typename OUTPUT::chan_out_internal;

        chan<T, POISONABLE> _chan; //<! Internal channel implementation.

        INPUT _in; //<! The input end of the channel.

        OUTPUT _out; //<! The output end of the channel.

    public:
        /*!
         * \brief Creates a new busy one2one channel
         *
         * \param[in] immunity The poison immunity level that the channel has.
         */
        busy_one2one_chan(unsigned int immunity = 0) noexcept
        : _chan(busy_chan<T, POISONABLE>()),
          _in(std::shared_ptr<INPUT_IMPL>(new INPUT_IMPL(_chan, immunity))),
          _out(std::shared_ptr<OUTPUT_IMPL>(new OUTPUT_IMPL(_chan, immunity)))
        {
        }

        /*!
         * \brief Creates a new buffered busy one2one channel
         *
         * \param[in] buffer The buffer to use with the channel.
         * \param[in] immunity The poison immunity level that the channel has.
         */
        busy_one2one_chan(chan_data_store<T> &buffer, unsigned int immunity = 0) noexcept
        : _chan(busy_buffered_chan<T, POISONABLE>(buffer)),
          _in(std::shared_ptr<INPUT_IMPL>(new INPUT_IMPL(_chan, immunity))),
          _out(std::shared_ptr<OUTPUT_IMPL>(new OUTPUT_IMPL(_chan, immunity)))
        {
        }

        /*!
         * \brief Gets the input end of the one2one channel.
         *
         * \return The input end of the channel.
         */
        alting_chan_in<T, POISONABLE> in() const noexcept { return _in; }

        /*!
         * \brief Gets the output end of the one2one channel.
         *
         * \return The output end of the channel.
         */
        chan_out<T, POISONABLE> out() const noexcept { return _out; }

        /*!
         * \brief Conversion operator.  Implicitly gets input end.
         *
         * \return The input end of the channel.
         */
        operator alting_chan_in<T, POISONABLE>() const noexcept { return _in; }

        /*!
         * \brief Conversion operator.  Implicitly gets output end.
         *
         * \return The output end of the channel.
         */
        operator chan_out<T, POISONABLE>() const noexcept { return _out; }

        /*!
         * \brief Performs a read on the channel.
         *
         * \return Value read from the channel.
         */
        T operator()() const noexcept
        {
            return _in.read();
        }

        /*!
         * \brief Performs a write on the channel.
         *
         * \param[in] value Value to write to the channel.
         */
        void operator()(T value) const noexcept
        {
            _out.write(value);
        }
    };

    // TODO : Add correct busy buffered channel.

    /*! \class one2any_chan
     * \brief A channel with a shared input and non-shared output end.
     *
     * \tparam T The type that the channel operates on.
     * \tparam POISONABLE Flag to indicate if the channel can be poisoned.
     *
     * \author Kevin Chalmers
     *
     * \date 02/05/2016
     */
    template<typename T, bool POISONABLE = false>
    class one2any_chan
    {
    private:
        // Type declarations used by channel
        using INPUT = shared_chan_in<T, POISONABLE>;
        using INPUT_IMPL = typename INPUT::shared_chan_in_internal;
        using OUTPUT = chan_out<T, POISONABLE>;
        using OUTPUT_IMPL = typename OUTPUT::chan_out_internal;

        chan<T, POISONABLE> _chan; //<! Pointer to the internal channel.

        INPUT _in; //<! The input end of the channel.

        OUTPUT _out; //<! The output end of the channel.

    public:
        /*!
         * \brief Creates a new one2any_chan.
         *
         * \param[in] immunity The poison immunity level of the channel.
         */
        one2any_chan(unsigned int immunity = 0) noexcept
        : _chan(basic_chan<T, POISONABLE>()),
          _in(std::shared_ptr<INPUT_IMPL>(new INPUT_IMPL(_chan, immunity))),
          _out(std::shared_ptr<OUTPUT_IMPL>(new OUTPUT_IMPL(_chan, immunity)))
        {
        }

        /*!
         * \brief Creates a new buffered one2any_chan.
         *
         * \param[in] buffer The buffer to use with the channel.
         * \param[in] immunity The poison immunity level of the channel.
         */
        one2any_chan(chan_data_store<T> &buffer, unsigned int immunity = 0) noexcept
        : _chan(buffered_chan<T, POISONABLE>(buffer)),
          _in(std::shared_ptr<INPUT_IMPL>(new INPUT_IMPL(_chan, immunity))),
          _out(std::shared_ptr<OUTPUT_IMPL>(new OUTPUT_IMPL(_chan, immunity)))
        {
        }

        /*!
         * \brief Gets the input end of the one2any channel.
         *
         * \return The input end of the channel.
         */
        shared_chan_in<T, POISONABLE> in() const noexcept { return _in; }

        /*!
         * \brief Gets the output end of the one2any channel.
         *
         * \return The output end of the channel.
         */
        chan_out<T, POISONABLE> out() const noexcept { return _out; }

        /*!
         * \brief Conversion operator.  Implicitly gets input end.
         *
         * \return The input end of the channel.
         */
        operator shared_chan_in<T, POISONABLE>() const noexcept { return _in; }

        /*!
         * \brief Conversion operator.  Implicitly gets the output end.
         *
         * \return The output end of the channel.
         */
        operator chan_out<T, POISONABLE>() const noexcept { return _out; }

        /*!
         * \brief Performs a read operation.
         *
         * \return The value read from the channel.
         */
        T operator()() const noexcept { return _in.read(); }

        /*!
         * \brief Performs a write operation.
         *
         * \param[in] value The value to write to the channel.
         */
        void operator()(T value) const noexcept { _out.write(value); }
    };

    /*! \class busy_one2any_chan
     * \brief A channel with a shared input and non-shared output end using busy semantics.
     *
     * \tparam T The type that the channel operates on.
     * \tparam POISONABLE Flag to indicate if the channel can be poisoned.
     *
     * \author Kevin Chalmers
     *
     * \date 19/10/2016
     */
    template<typename T, bool POISONABLE = false>
    class busy_one2any_chan
    {
    private:
        // Type declarations used by channel
        using INPUT = busy_shared_chan_in<T, POISONABLE>;
        using INPUT_IMPL = typename INPUT::busy_shared_chan_in_internal;
        using OUTPUT = chan_out<T, POISONABLE>;
        using OUTPUT_IMPL = typename OUTPUT::chan_out_internal;

        chan<T, POISONABLE> _chan; //<! Pointer to the internal channel.

        INPUT _in; //<! The input end of the channel.

        OUTPUT _out; //<! The output end of the channel.

    public:
        /*!
         * \brief Creates a new busy one2any_chan.
         *
         * \param[in] immunity The poison immunity level of the channel.
         */
        busy_one2any_chan(unsigned int immunity = 0) noexcept
        : _chan(busy_chan<T, POISONABLE>()),
          _in(std::shared_ptr<INPUT_IMPL>(new INPUT_IMPL(_chan, immunity))),
          _out(std::shared_ptr<OUTPUT_IMPL>(new OUTPUT_IMPL(_chan, immunity)))
        {
        }

        /*!
         * \brief Creates a new buffered busy one2any_chan.
         *
         * \param[in] buffer The buffer to use with the channel.
         * \param[in] immunity The poison immunity level of the channel.
         */
        busy_one2any_chan(chan_data_store<T> &buffer, unsigned int immunity = 0) noexcept
        : _chan(buffered_chan<T, POISONABLE>(buffer)),
          _in(std::shared_ptr<INPUT_IMPL>(new INPUT_IMPL(_chan, immunity))),
          _out(std::shared_ptr<OUTPUT_IMPL>(new OUTPUT_IMPL(_chan, immunity)))
        {
        }

        /*!
         * \brief Gets the input end of the busy one2any channel.
         *
         * \return The input end of the channel.
         */
        busy_shared_chan_in<T, POISONABLE> in() const noexcept { return _in; }

        /*!
         * \brief Gets the output end of the busy one2any channel.
         *
         * \return The output end of the channel.
         */
        chan_out<T, POISONABLE> out() const noexcept { return _out; }

        /*!
         * \brief Conversion operator.  Implicitly gets input end.
         *
         * \return The input end of the channel.
         */
        operator busy_shared_chan_in<T, POISONABLE>() const noexcept { return _in; }

        /*!
         * \brief Conversion operator.  Implicitly gets the output end.
         *
         * \return The output end of the channel.
         */
        operator chan_out<T, POISONABLE>() const noexcept { return _out; }

        /*!
         * \brief Performs a read operation.
         *
         * \return The value read from the channel.
         */
        T operator()() const noexcept { return _in.read(); }

        /*!
         * \brief Performs a write operation.
         *
         * \param[in] value The value to write to the channel.
         */
        void operator()(T value) const noexcept { _out.write(value); }
    };

    /*! \class any2one_chan
     * \brief A channel with an alting input end and a shared output end.
     *
     * \tparam T The type that the channel operates on.
     * \tparam POISONABLE Flag to indicate if the channel can be poisoned.
     *
     * \author Kevin Chalmers
     *
     * \date 02/05/2016
     */
    template<typename T, bool POISONABLE = false>
    class any2one_chan
    {
    private:
        // Type declarations used by the channel.
        using INPUT = alting_chan_in<T, POISONABLE>;
        using INPUT_IMPL = typename INPUT::alting_chan_in_internal;
        using OUTPUT = shared_chan_out<T, POISONABLE>;
        using OUTPUT_IMPL = typename OUTPUT::shared_chan_out_internal;

        chan<T, POISONABLE> _chan; //<! Pointer to the internal channel object.

        INPUT _in; //<! The input end of the channel.

        OUTPUT _out; //<! The output end of the channel.

    public:
        /*!
         * \brief Creates a new any2one channel.
         *
         * \param[in] immunity The poison immunity level of the channel.
         */
        any2one_chan(unsigned int immunity = 0) noexcept
        : _chan(basic_chan<T, POISONABLE>()),
          _in(std::shared_ptr<INPUT_IMPL>(new INPUT_IMPL(_chan, immunity))),
          _out(std::shared_ptr<OUTPUT_IMPL>(new OUTPUT_IMPL(_chan, immunity)))
        {
        }

        /*!
         * \brief Creates a new buffered any2one channel.
         *
         * \param[in] buffer The buffer used internally by the channel.
         * \param[in] immunity The poison immunity level of the channel.
         */
        any2one_chan(chan_data_store<T> &buffer, unsigned int immunity = 0) noexcept
        : _chan(buffered_chan<T, POISONABLE>(buffer)),
          _in(std::shared_ptr<INPUT_IMPL>(new INPUT_IMPL(_chan, immunity))),
          _out(std::shared_ptr<OUTPUT_IMPL>(new OUTPUT_IMPL(_chan, immunity)))
        {
        }

        /*!
         * \brief Gets the input end of the channel.
         *
         * \return The input end of the channel.
         */
        alting_chan_in<T, POISONABLE> in() const noexcept { return _in; }

        /*!
         * \brief Gets the output end of the channel.
         *
         * \return The output end of the channel.
         */
        shared_chan_out<T, POISONABLE> out() const noexcept { return _out; }

        /*!
         * \brief Converstion operator.  Implicitly gets the input end.
         *
         * \return The input end of the channel.
         */
        operator alting_chan_in<T, POISONABLE>() const noexcept { return _in; }

        /*!
         * \brief Converstion operator.  Implicitly gets the output end.
         *
         * \return The output end of the channel.
         */
        operator shared_chan_out<T, POISONABLE>() const noexcept { return _out; }

        /*!
         * \brief Performs a read operation on the channel.
         *
         * \return The value read from the channel.
         */
        T operator()() const noexcept { return _in.read(); }

        /*!
         * \brief Performs a write operation on the channel.
         *
         * \param[in] value The value to write to the channel.
         */
        void operator()(T value) const noexcept { _out.write(value); }
    };

    /*! \class busy_any2one_chan
     * \brief A busy channel with an alting input end and a shared output end.
     *
     * \tparam T The type that the channel operates on.
     * \tparam POISONABLE Flag to indicate if the channel can be poisoned.
     *
     * \author Kevin Chalmers
     *
     * \date 21/10/2016
     */
    template<typename T, bool POISONABLE = false>
    class busy_any2one_chan
    {
    private:
        // Type declarations used by the channel.
        using INPUT = alting_chan_in<T, POISONABLE>;
        using INPUT_IMPL = typename INPUT::alting_chan_in_internal;
        using OUTPUT = busy_shared_chan_out<T, POISONABLE>;
        using OUTPUT_IMPL = typename OUTPUT::busy_shared_chan_out_internal;

        chan<T, POISONABLE> _chan; //<! Pointer to the internal channel object.

        INPUT _in; //<! The input end of the channel.

        OUTPUT _out; //<! The output end of the channel.

    public:
        /*!
         * \brief Creates a new busy any2one channel.
         *
         * \param[in] immunity The poison immunity level of the channel.
         */
        busy_any2one_chan(unsigned int immunity = 0) noexcept
        : _chan(busy_chan<T, POISONABLE>()),
          _in(std::shared_ptr<INPUT_IMPL>(new INPUT_IMPL(_chan, immunity))),
          _out(std::shared_ptr<OUTPUT_IMPL>(new OUTPUT_IMPL(_chan, immunity)))
        {
        }

        /*!
         * \brief Creates a new buffered any2one channel.
         *
         * \param[in] buffer The buffer used internally by the channel.
         * \param[in] immunity The poison immunity level of the channel.
         */
        busy_any2one_chan(chan_data_store<T> &buffer, unsigned int immunity = 0) noexcept
        : _chan(buffered_chan<T, POISONABLE>(buffer)),
          _in(std::shared_ptr<INPUT_IMPL>(new INPUT_IMPL(_chan, immunity))),
          _out(std::shared_ptr<OUTPUT_IMPL>(new OUTPUT_IMPL(_chan, immunity)))
        {
        }

        /*!
         * \brief Gets the input end of the channel.
         *
         * \return The input end of the channel.
         */
        alting_chan_in<T, POISONABLE> in() const noexcept { return _in; }

        /*!
         * \brief Gets the output end of the channel.
         *
         * \return The output end of the channel.
         */
        busy_shared_chan_out<T, POISONABLE> out() const noexcept { return _out; }

        /*!
         * \brief Converstion operator.  Implicitly gets the input end.
         *
         * \return The input end of the channel.
         */
        operator alting_chan_in<T, POISONABLE>() const noexcept { return _in; }

        /*!
         * \brief Converstion operator.  Implicitly gets the output end.
         *
         * \return The output end of the channel.
         */
        operator busy_shared_chan_out<T, POISONABLE>() const noexcept { return _out; }

        /*!
         * \brief Performs a read operation on the channel.
         *
         * \return The value read from the channel.
         */
        T operator()() const noexcept { return _in.read(); }

        /*!
         * \brief Performs a write operation on the channel.
         *
         * \param[in] value The value to write to the channel.
         */
        void operator()(T value) const noexcept { _out.write(value); }
    };

    /*! \class any2any_chan
     * \brief A channel with shared input and output ends.
     *
     * \tparam T The type that the channel operates on.
     * \tparam POISONABLE Flag to indicate if the channel can be poisoned.
     *
     * \author Kevin Chalmers
     *
     * \date 02/05/2016
     */
    template<typename T, bool POISONABLE = false>
    class any2any_chan
    {
    private:
        // Types declarations used by the class.
        using INPUT = shared_chan_in<T, POISONABLE>;
        using INPUT_IMPL = typename INPUT::shared_chan_in_internal;
        using OUTPUT = shared_chan_out<T, POISONABLE>;
        using OUTPUT_IMPL = typename OUTPUT::shared_chan_out_internal;

        chan<T, POISONABLE> _chan; //<! Pointer to the internal channel implementation.

        INPUT _in; //<! The input end of the channel.

        OUTPUT _out; //<! The output end of the channel.

    public:
        /*!
         * \brief Creates a new any2any channel.
         *
         * \param[in] immunity The poison immunity level of the channel.
         */
        any2any_chan(unsigned int immunity = 0) noexcept
        : _chan(basic_chan<T, POISONABLE>()),
          _in(std::shared_ptr<INPUT_IMPL>(new INPUT_IMPL(_chan, immunity))),
          _out(std::shared_ptr<OUTPUT_IMPL>(new OUTPUT_IMPL(_chan, immunity)))
        {
        }

        /*!
         * \brief Creates a new buffered any2any channel.
         *
         * \param[in] buffer The buffer used internally by the channel.
         * \param[in] immunity The poison immunity level of the channel.
         */
        any2any_chan(chan_data_store<T> &buffer, unsigned int immunity = 0) noexcept
        : _chan(buffered_chan<T, POISONABLE>(buffer)),
          _in(std::shared_ptr<INPUT_IMPL>(new INPUT_IMPL(_chan, immunity))),
          _out(std::shared_ptr<OUTPUT_IMPL>(new OUTPUT_IMPL(_chan, immunity)))
        {
        }

        /*!
         * \brief Gets the input end of the channel.
         *
         * \return The input end of the channel.
         */
        shared_chan_in<T, POISONABLE> in() const noexcept { return _in; }

        /*!
         * \brief Gets the output end of the channel.
         *
         * \return The output end of the channel.
         */
        shared_chan_out<T, POISONABLE> out() const noexcept { return _out; }

        /*!
         * \brief Conversion operator.  Implicitly gets the input end.
         *
         * \return The input end of the channel.
         */
        operator shared_chan_in<T, POISONABLE>() const noexcept { return _in; }

        /*!
         * \brief Conversion operator.  Implicitly gets the output end.
         *
         * \return The output end of the channel.
         */
        operator shared_chan_out<T, POISONABLE>() const noexcept { return _out; }

        /*!
         * \brief Performs a read operation on the channel.
         *
         * \return The value read from the channel.
         */
        T operator()() const noexcept { return _in.read(); }

        /*!
         * \brief Performs a write operation on the channel.
         *
         * \param[in] value The value written to the channel.
         */
        void operator()(T value) const noexcept { _out.write(value); }
    };

    /*! \class busy_any2any_chan
     * \brief A busy channel with shared input and output ends.
     *
     * \tparam T The type that the channel operates on.
     * \tparam POISONABLE Flag to indicate if the channel can be poisoned.
     *
     * \author Kevin Chalmers
     *
     * \date 21/10/2016
     */
    template<typename T, bool POISONABLE = false>
    class busy_any2any_chan
    {
    private:
        // Types declarations used by the class.
        using INPUT = busy_shared_chan_in<T, POISONABLE>;
        using INPUT_IMPL = typename INPUT::busy_shared_chan_in_internal;
        using OUTPUT = busy_shared_chan_out<T, POISONABLE>;
        using OUTPUT_IMPL = typename OUTPUT::busy_shared_chan_out_internal;

        chan<T, POISONABLE> _chan; //<! Pointer to the internal channel implementation.

        INPUT _in; //<! The input end of the channel.

        OUTPUT _out; //<! The output end of the channel.

    public:
        /*!
         * \brief Creates a new busy any2any channel.
         *
         * \param[in] immunity The poison immunity level of the channel.
         */
        busy_any2any_chan(unsigned int immunity = 0) noexcept
        : _chan(busy_chan<T, POISONABLE>()),
          _in(std::shared_ptr<INPUT_IMPL>(new INPUT_IMPL(_chan, immunity))),
          _out(std::shared_ptr<OUTPUT_IMPL>(new OUTPUT_IMPL(_chan, immunity)))
        {
        }

        /*!
         * \brief Creates a new busy buffered any2any channel.
         *
         * \param[in] buffer The buffer used internally by the channel.
         * \param[in] immunity The poison immunity level of the channel.
         */
        busy_any2any_chan(chan_data_store<T> &buffer, unsigned int immunity = 0) noexcept
        : _chan(buffered_chan<T, POISONABLE>(buffer)),
          _in(std::shared_ptr<INPUT_IMPL>(new INPUT_IMPL(_chan, immunity))),
          _out(std::shared_ptr<OUTPUT_IMPL>(new OUTPUT_IMPL(_chan, immunity)))
        {
        }

        /*!
         * \brief Gets the input end of the channel.
         *
         * \return The input end of the channel.
         */
        busy_shared_chan_in<T, POISONABLE> in() const noexcept { return _in; }

        /*!
         * \brief Gets the output end of the channel.
         *
         * \return The output end of the channel.
         */
        busy_shared_chan_out<T, POISONABLE> out() const noexcept { return _out; }

        /*!
         * \brief Conversion operator.  Implicitly gets the input end.
         *
         * \return The input end of the channel.
         */
        operator busy_shared_chan_in<T, POISONABLE>() const noexcept { return _in; }

        /*!
         * \brief Conversion operator.  Implicitly gets the output end.
         *
         * \return The output end of the channel.
         */
        operator busy_shared_chan_out<T, POISONABLE>() const noexcept { return _out; }

        /*!
         * \brief Performs a read operation on the channel.
         *
         * \return The value read from the channel.
         */
        T operator()() const noexcept { return _in.read(); }

        /*!
         * \brief Performs a write operation on the channel.
         *
         * \param[in] value The value written to the channel.
         */
        void operator()(T value) const noexcept { _out.write(value); }
    };
}

#endif //CPP_CSP_CHAN_H
