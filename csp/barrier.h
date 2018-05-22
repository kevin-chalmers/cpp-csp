//
// Created by kevin on 15/04/16.
//

#ifndef CPP_CSP_BARRIER_H
#define CPP_CSP_BARRIER_H

#include <mutex>
#include <condition_variable>
#include <memory>
#include <atomic>
#include <boost/fiber/all.hpp>

namespace csp
{
    /*! \class barrier
     * \brief Allows synchronization between a set of processes.
     *
     * \author Kevin Chalmers
     * \date 15/4/2016
     */
    class barrier
    {
    protected:
        /*! \class barrier_internal
         * \brief Internal representation of a barrier
         *
         * \author Kevin Chalmers
         * \date 15/4/2016
         */
        class barrier_internal
        {
        private:
            unsigned int _enrolled = 0; //<! Number of processes enrolled on the barrier

            unsigned int _count_down = 0; //<! Number of processes still to be synchronized

            std::mutex _mut; //<! Mutex used to control access to the barrier

            std::condition_variable _cond; //<! Condition variable used to control synchronized communication within the barrier

        public:
            /*!
             * \brief Creates a new barrier with 0 enrolled processes
             */
            barrier_internal() noexcept
            {
            }

            /*!
             * \brief Creates a new barrier with enrolled processes
             *
             * \param[in] enrolled Number of processes to enrol
             */
            barrier_internal(unsigned int enrolled) noexcept
            : _enrolled(enrolled), _count_down(enrolled)
            {
            }

            /*!
             * \brief Virtual destructor
             */
            virtual ~barrier_internal() { }

            /*!
             * \brief Syncs a process with the barrier
             */
            virtual void sync() noexcept
            {
                // Lock the mutex
                std::unique_lock<std::mutex> lock(_mut);

                // Decrement countdown
                --_count_down;

                // Check if last process has synchronized
                if (_count_down > 0)
                    _cond.wait(lock);
                else
                {
                    _count_down = _enrolled;
                    _cond.notify_all();
                }
            }

            /*!
             * \brief Enrolls another process with the barrier
             */
            virtual void enroll() noexcept(false)
            {
                // Lock the mutex
                std::lock_guard<std::mutex> lock(_mut);
                // Increment enrolled and count down
                ++_enrolled;
                ++_count_down;
            }

            /*!
             * \brief Resigns a process from the barrier
             */
            virtual void resign() noexcept(false)
            {
                // Lock the mutex
                std::lock_guard<std::mutex> lock(_mut);
                // Decrement enrolled
                --_enrolled;
                --_count_down;
                // Check if we should complete the synchronization
                if (_count_down == 0)
                {
                    _count_down = _enrolled;
                    _cond.notify_all();
                }
            }

            /*!
             * \brief Resets the number of processes enrolled with the barrier
             *
             * \param[in] enrolled The number of processes to be enrolled with the barrier.
             */
            virtual void reset(unsigned int enrolled) noexcept
            {
                // Lock the barrier
                std::lock_guard<std::mutex> lock(_mut);
                // Reset enrolled and count down
                _enrolled = enrolled;
                _count_down = enrolled;
            }
        };

        std::shared_ptr<barrier_internal> _internal = nullptr; //<! Pointer to internal representation of the barrier

        /*!
         * \brief Used by subclasses to initialise the barrier.
         *
         * \param[in] internal Pointer to the internal barrier.
         */
        barrier(std::shared_ptr<barrier_internal> internal) noexcept
        : _internal(internal)
        {
        }

    public:
        /*!
         * \brief Creates a new barrier with 0 enrolled processes.
         */
        barrier() noexcept
        : _internal(std::shared_ptr<barrier_internal>(new barrier_internal()))
        {
        }

        /*!
         * \brief Creates a new barrier with enrolled processes.
         *
         * \param[in] size The number of processes to be enrolled with the barrier.
         */
        barrier(unsigned int size) noexcept
        : _internal(std::shared_ptr<barrier_internal>(new barrier_internal(size)))
        {
        }

        /*!
         * \brief Syncs a process with the barrier.
         */
        void sync() const noexcept { _internal->sync(); }

        /*!
         * \brief Operator overload - calls sync on the barrier.
         */
        void operator()() const noexcept { _internal->sync(); }

        /*!
         * \brief Enrolls another process with the barrier.
         */
        void enroll() const noexcept { _internal->enroll(); }

        /*!
         * \brief Resigns a process from the barrier.
         */
        void resign() const noexcept { _internal->resign(); }

        /*!
         * \brief Resets the number of processes enrolled.
         *
         * \param[in] enrolled The number of processes to be enrolled with the barrier.
         */
        void reset(unsigned int enrolled) const noexcept { _internal->reset(enrolled); }
    };

    class fiber_barrier : public barrier
    {
    protected:
        class fiber_barrier_internal : public barrier::barrier_internal
        {
        private:
            unsigned int _enrolled = 0;

            unsigned int _count_down = 0;

            boost::fibers::mutex _mut;

            boost::fibers::condition_variable _cond;

        public:
            fiber_barrier_internal() { }

            fiber_barrier_internal(unsigned int enrolled)
            : _enrolled(enrolled), _count_down(enrolled)
            {
            }

            ~fiber_barrier_internal() = default;

            void sync() noexcept final
            {
                std::unique_lock<boost::fibers::mutex> lock(_mut);
                --_count_down;
                if (_count_down > 0)
                    _cond.wait(lock);
                else
                {
                    _count_down = _enrolled;
                    _cond.notify_all();
                }
            }

            void enroll() final
            {
                std::lock_guard<boost::fibers::mutex> lock(_mut);
                ++_enrolled;
                ++_count_down;
            }

            void resign() final
            {
                std::lock_guard<boost::fibers::mutex> lock(_mut);
                --_enrolled;
                --_count_down;
                if (_count_down == 0)
                {
                    _count_down = _enrolled;
                    _cond.notify_all();
                }
            }

            void reset(unsigned int enrolled) noexcept final
            {
                std::lock_guard<boost::fibers::mutex> lock(_mut);
                _enrolled = enrolled;
                _count_down = enrolled;
            }
        };

        std::shared_ptr<fiber_barrier_internal> _internal = nullptr;

        fiber_barrier(std::shared_ptr<fiber_barrier_internal> internal)
        : _internal(internal), barrier(internal)
        {
        }

    public:
        fiber_barrier()
        : _internal(std::shared_ptr<fiber_barrier_internal>(new fiber_barrier_internal()))
        {
        }

        fiber_barrier(unsigned int size)
        : _internal(std::shared_ptr<fiber_barrier_internal>(new fiber_barrier_internal(size)))
        {
        }
    };

    /*! \class busy_barrier
     * \brief Allows synchronization between a set of processes.  Uses atomics for busy semantics.
     *
     * \author Kevin Chalmers
     * \date 21/10/2016
     */
    class busy_barrier : public barrier
    {
    protected:
        /*! \class busy_barrier_internal
         * \brief Internal representation of a busy barrier
         *
         * \author Kevin Chalmers
         * \date 21/10/2016
         */
        class busy_barrier_internal : public barrier::barrier_internal
        {
        private:

            std::atomic<unsigned int> _enrolled; //<! Number of processes enrolled on the barrier

            std::atomic<unsigned int> _iteration; //<! Iteration cycle currently completed

            std::atomic<unsigned int> _syncing; //<! Number of processes currently finalising the sync

        public:
            /*!
             * \brief Creates a new busy barrier with 0 enrolled processes
             */
            busy_barrier_internal() noexcept
            : _enrolled(0), _iteration(0), _syncing(0)
            {
            }

            /*!
             * \brief Creates a new busy barrier with enrolled processes
             *
             * \param[in] enrolled Number of processes to enrol
             */
            busy_barrier_internal(unsigned int enrolled) noexcept
            : _enrolled(enrolled), _iteration(0), _syncing(0)
            {
            }

            /*!
             * \brief Virtual destructor
             */
            virtual ~busy_barrier_internal() { }

            /*!
             * \brief Syncs a process with the barrier
             */
            virtual void sync() noexcept
            {
                // Get current iteration
                auto iter = _iteration.load(std::memory_order_acquire);
                // Now increment syncing processes, and check if last has synced.
                if (_syncing.fetch_add(1, std::memory_order_acq_rel) == _enrolled.load(std::memory_order_acquire) - 1)
                {
                    // Reset the number of processes syncing
                    _syncing.store(0, std::memory_order_relaxed);
                    // Increment iteration, releasing all spinning processes
                    _iteration.fetch_add(1, std::memory_order_release);
                }
                else
                    while (_iteration.load(std::memory_order_acquire) == iter);
            }

            /*!
             * \brief Enrolls another process with the barrier
             */
            virtual void enroll() noexcept(false)
            {
                // Increment enrolled and countdown
                _enrolled.fetch_add(1, std::memory_order_release);
            }

            /*!
             * \brief Resigns a process from the barrier
             */
            virtual void resign() noexcept(false)
            {
                // First get current iteration - may need it
                auto iter = _iteration.load(std::memory_order_acquire);
                // Now decrement the enrolled count and check if we should notify
                if (_enrolled.fetch_sub(1, std::memory_order_acq_rel) == _enrolled.load(std::memory_order_acquire) - 1)
                {
                    // Need to notify waiting processes that no other process will sync
                    _syncing.store(0, std::memory_order_relaxed);
                    _iteration.fetch_add(1, std::memory_order_release);
                }
                // Otherwise we have nothing to do
            }

            /*!
             * \brief Resets the number of processes enrolled with the barrier
             *
             * \param[in] enrolled The number of processes to be enrolled with the barrier.
             */
            virtual void reset(unsigned int enrolled) noexcept
            {
                // Reset enrolled and count down
                _enrolled.store(enrolled);
            }
        };

        std::shared_ptr<busy_barrier_internal> _internal = nullptr; //<! Pointer to internal representation of the barrier

        /*!
         * \brief Used by subclasses to initialise the barrier.
         *
         * \param[in] internal Pointer to the internal barrier.
         */
        busy_barrier(std::shared_ptr<busy_barrier_internal> internal) noexcept
        : _internal(internal), barrier(internal)
        {
        }

    public:
        /*!
         * \brief Creates a new busy barrier with 0 enrolled processes.
         */
        busy_barrier() noexcept
        : _internal(std::shared_ptr<busy_barrier_internal>(new busy_barrier_internal()))
        {
           barrier::_internal = _internal;
        }

        /*!
         * \brief Creates a new busy barrier with enrolled processes.
         *
         * \param[in] size The number of processes to be enrolled with the barrier.
         */
        busy_barrier(unsigned int size) noexcept
        : _internal(std::shared_ptr<busy_barrier_internal>(new busy_barrier_internal(size)))
        {
            barrier::_internal = _internal;
        }
    };
}

#endif //CPP_CSP_BARRIER_H
