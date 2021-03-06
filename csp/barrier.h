//
// Created by kevin on 15/04/16.
//

#ifndef CPP_CSP_BARRIER_H
#define CPP_CSP_BARRIER_H

#include <mutex>
#include <condition_variable>
#include <memory>

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
}

#endif //CPP_CSP_BARRIER_H
