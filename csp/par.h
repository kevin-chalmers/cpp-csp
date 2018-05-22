//
// Created by kevin on 28/04/16.
//

#ifndef CPP_CSP_PAR_H
#define CPP_CSP_PAR_H

#include <memory>
#include <initializer_list>
#include <vector>
#include <functional>
#include <thread>
#include <mutex>
#include <boost/fiber/all.hpp>
#include <set>
#include "process.h"
#include "barrier.h"

namespace csp
{
    /*! \class par
     *
     * \brief Runs a collection of processes in parallel.
     *
     * \author Kevin Chalmers
     *
     * \date 28/04/2016
     */
    class par : public process
    {
    private:
        /*! \class par_thread
         *
         * \brief Helper class used to wrap a thread in a reusable manner.
         *
         * \author Kevin Chalmers
         *
         * \date 28/04/2016
         */
        class par_thread
        {
        public:
            std::function<void()> _process; //<! The function to run in the thread.

            std::shared_ptr<std::thread> _thread = nullptr;  //<! Thread to run the process in.

            barrier _bar; //<! The barrier used to synchronise parallel completion.

            barrier _park = barrier(2); //<! The barrier used to coordinate completion of this thread.

            bool _running = true; //<! Flag used to determine if the thread is running.

            /*!
             * \brief Creates a par thread
             */
            par_thread() noexcept { }

            /*!
             * \brief Creates a par thread from a function and barrier
             *
             * \param[in] proc The function to run in the thread.
             * \param[in] bar The barrier used to synchronise the parallel.
             */
            par_thread(std::function<void()> &proc, barrier &bar) noexcept
            : _process(proc), _bar(bar)
            {
            }

            /*!
             * \brief Destroys the par thread.
             */
            ~par_thread() noexcept;

            // Delete copy and move constructors
            par_thread(const par_thread &other) = delete;
            par_thread(par_thread &&rhs) = delete;

            // Delete assignment operators
            par_thread& operator=(const par_thread &other) = delete;
            par_thread& operator=(par_thread &&rhs) = delete;

            /*!
             * \brief Resets the par thread, changing the process and barrier.
             *
             * \param[in] proc The process to swap to.
             * \param[in] bar The barrier to sync the par with.
             */
            void reset(std::function<void()> proc, barrier bar) noexcept
            {
                _process = proc;
                _bar = bar;
                _running = true;
            }

            /*!
             * \brief Terminates the running thread.
             */
            void terminate() noexcept
            {
                _running = false;
                _park();
            }

            /*!
             * \brief Releases the currently terminated thread.
             */
            void release() noexcept
            {
                _park();
            }

            /*!
             * \brief Runs the process by executing it in the running thread.
             */
            void run() noexcept;

            /*!
             * \brief Starts the thread.
             */
            void start() noexcept;

            /*!
             * \brief Operator overload to run the thread.
             */
            void operator()() noexcept { run(); }
        };

        /*! \class par_internal
         *
         * \brief Internal representation of a par.
         *
         * \author Kevin Chalmers
         *
         * \date 28/04/2016
         */
        class par_internal
        {
        public:

            static std::set<std::shared_ptr<std::thread>> _all_threads; //<! The set of all threads in the CSP framework.

            static std::shared_ptr<std::mutex> _all_threads_lock; //<! Lock to control access to all threads.

            std::mutex _mut; //<! Mutex to control access to the parallel.

            std::vector<std::function<void()>> _processes;  //<! The vector of functions to run in parallel.

            std::vector<std::shared_ptr<par_thread>> _threads; //<! The vector of threads associated with this parallel.

            barrier _barrier = barrier(0); //<! Barrier used to synchronise the processes running in the par.

            bool _process_changed = true; //<! Flag used to indicate whether the process list has changed.

            /*!
             * \brief Creates an empty internal par object.
             */
            par_internal() noexcept { }

            /*!
             * \brief Creates an internal par object with the given processes
             *
             * \param[in] procs The list of processes to run in parallel.
             */
            par_internal(std::initializer_list<std::function<void()>> &&procs) noexcept
            : _processes(std::forward<std::initializer_list<std::function<void()>>>(procs))
            {
            }

            /*!
             * \brief Creates an internal par object with the given processes.
             *
             * \param[in] procs Vector of processes to run in parallel.
             */
            par_internal(std::vector<std::function<void()>> &procs) noexcept
            : _processes(procs)
            {
            }

            /*!
             * \brief Destroys the internal par object.
             */
            ~par_internal() noexcept
            {
                // Release all threads
                release_all_threads();
            }

            /*!
             * \brief Releases all the threads used by this par.
             */
            void release_all_threads() noexcept
            {
                // Lock the mutex
                std::lock_guard<std::mutex> lock(_mut);
                // Iterate through the threads
                for (auto &t : _threads)
                {
                    t->terminate();
                    t->_thread->join();
                }
                // Set the processes changed flag
                _process_changed = true;
                // Clear the threads
                _threads.clear();
            }

            /*!
             * \brief Runs the process in parallel.
             */
            void run() noexcept
            {
                // Flag to indicate if this is an empty run
                bool empty_run = true;
                // Process that the main thread running the par executes
                std::function<void()> my_process;
                // Lock the mutex
                std::unique_lock<std::mutex> lock(_mut);
                // Check if processes exist
                if (_processes.size() > 0)
                {
                    // Set empty run to false
                    empty_run = false;
                    // Get the process to run by main thread
                    my_process = _processes[_processes.size() - 1];
                    // Check if processes have changed
                    if (_process_changed)
                    {
                        // Set barrier
                        _barrier.reset(static_cast<unsigned int>(_processes.size()));
                        // If the threads is less than number required add to the threads
                        if (_threads.size() < _processes.size() - 1)
                        {
                            // Loop adding threads to the thread vector
                            for (unsigned int i = 0; i < _threads.size(); ++i)
                            {
                                // Reset the thread
                                _threads[i]->reset(_processes[i], _barrier);
                                // Allow it to continue executing
                                _threads[i]->release();
                            }
                            // Add the new required threads to the thread vector
                            for (unsigned int i = static_cast<unsigned int>(_threads.size()); i < _processes.size() - 1; ++i)
                            {
                                // Add a new par thread
                                _threads.push_back(std::shared_ptr<par_thread>(new par_thread(_processes[i], _barrier)));
                                // Start the thread
                                _threads[i]->start();
                            }
                        }
                        else
                        {
                            // Resize the vector
                            _threads.resize(_processes.size() - 1);
                            // Set the threads
                            for (unsigned int i = 0; i < _processes.size() - 1; ++i)
                            {
                                // Reset thread
                                _threads[i]->reset(_processes[i], _barrier);
                                // Release thread to allow it to continue
                                _threads[i]->release();
                            }
                        }
                        // Set processes changed flag
                        _process_changed = false;
                    }
                    else
                    {
                        // Release all threads to continue running
                        for (unsigned int i = 0; i < _processes.size() - 1; ++i)
                            _threads[i]->release();
                    }
                }
                // Check if empty run
                if (!empty_run)
                {
                    // Run process
                    my_process();
                    // Sync with barrier
                    _barrier();
                }
            }

            /*!
             * \brief Adds a thread to the list of all threads associated with the framework.
             *
             * \param[in] thread The thread to add to the list of all threads.
             */
            static void add_to_all_threads(std::shared_ptr<std::thread> thread) noexcept
            {
                // Lock all threads
                std::unique_lock<std::mutex> lock(*_all_threads_lock);
                // Add to all threads
                _all_threads.emplace(thread);
            }

            /*!
             * \brief Removes a thread from the list of all threads associated with the framework.
             *
             * \param[in] thread The thread to remove from the list of all threads.
             */
            static void remove_from_all_threads(std::shared_ptr<std::thread> thread) noexcept
            {
                // Lock all threads
                std::unique_lock<std::mutex> lock(*_all_threads_lock);
                // Remove thread
                _all_threads.erase(thread);
            }


        };

        std::shared_ptr<par_internal> _internal = nullptr; //<! Pointer to the internal representation

    public:
        /*!
         * \brief Creates a new par object with no processes.
         */
        par() noexcept
        : _internal(std::shared_ptr<par_internal>(new par_internal()))
        {
        }

        /*!
         * \brief Creates a new par object with given processes.
         *
         * \param[in] procs The processes used to create the par.
         */
        par(std::initializer_list<std::function<void()>> &&procs) noexcept
        : _internal(std::shared_ptr<par_internal>(new par_internal(std::forward<std::initializer_list<std::function<void()>>>(procs))))
        {
        }

        /*!
         * \brief Creates a new par object with given processes.
         *
         * \param[in] procs The processes used to create the par.
         */
        par(std::vector<std::function<void()>> &procs) noexcept
        : _internal(std::shared_ptr<par_internal>(new par_internal(procs)))
        {
        }

        /*!
         * \brief Creates a new par object with the given range of processes
         *
         * \param[in] begin The start of the range.
         * \param[in] end The end of the range.
         */
        template<typename RanIt>
        par(RanIt begin, RanIt end) noexcept
        {
            static_assert(std::iterator_traits<RanIt>::value_type == typeid(std::function<void()>), "par can only take collections of functions");
            auto procs = std::vector<std::function<void()>>(begin, end);
            _internal = std::shared_ptr<par_internal>(new par_internal(procs));
        }

        /*!
         * \brief Copy constructor.
         *
         * \param[in] other The par to copy.
         */
        par(const par &other) noexcept = default;

        /*!
         * \brief Move constructor.
         *
         * \param[in] rhs The par to copy.
         */
        par(par &&rhs) noexcept = default;

        /*!
         * \brief Destroys the par object.
         */
        ~par() noexcept { }

        /*!
         * \brief Copy assignment operator.
         *
         * \param[in] other The par to copy.
         *
         * \return A copy of the par.
         */
        par& operator=(const par &other) noexcept = default;

        /*!
         * \brief Move assignment operator.
         *
         * \param[in] rhs The par to copy.
         *
         * \return A copy of the par.
         */
        par& operator=(par &&rhs) noexcept = default;

        /*!
         * \brief Executes the par.
         */
        void run() noexcept { _internal->run(); }
    };

    // Initialise the all threads set
    std::set<std::shared_ptr<std::thread>> par::par_internal::_all_threads = std::set<std::shared_ptr<std::thread>>();

    // Initialise all threads lock
    std::shared_ptr<std::mutex> par::par_internal::_all_threads_lock = std::make_shared<std::mutex>();

    par::par_thread::~par_thread() noexcept
    {
        // Remove thread from the framework
        par_internal::remove_from_all_threads(_thread);
    }

    void par::par_thread::run() noexcept
    {
        // Loop while running
        while (_running)
        {
            // Run the process
            _process();
            // Sync on barrier
            _bar();
            // Sync on park
            _park();
        }
        // Remove from all threads
        par_internal::remove_from_all_threads(_thread);
    }

    void par::par_thread::start() noexcept
    {
        // Create thread
        _thread = std::make_shared<std::thread>(&par_thread::run, this);
        // Add to all threads
        par_internal::add_to_all_threads(_thread);
        // Set running to true
        _running = true;
    }

    class fiber_par : public process
    {
    private:
        class par_fiber
        {
        public:
            std::function<void()> _process;

            std::shared_ptr<boost::fibers::fiber> _fiber = nullptr;

            fiber_barrier _bar;

            fiber_barrier _park = fiber_barrier(2);

            bool _running = true;

            par_fiber() { }

            par_fiber(std::function<void()> &proc, fiber_barrier &bar)
            : _process(proc), _bar(bar)
            {
            }

            ~par_fiber();

            par_fiber(const par_fiber&) = delete;
            par_fiber(par_fiber&&) = delete;

            void reset(std::function<void()> proc, fiber_barrier bar) noexcept
            {
                _process = proc;
                _bar = bar;
                _running = true;
            }

            void terminate() noexcept
            {
                _running = false;
                _park();
            }

            void release() noexcept
            {
                _park();
            }

            void run() noexcept;

            void start() noexcept;

            void operator()() noexcept { run(); }
        };

        class fiber_par_internal
        {
        public:
            thread_local static std::set<std::shared_ptr<boost::fibers::fiber>> _all_fibers;

            thread_local static std::shared_ptr<boost::fibers::mutex> _all_fibers_lock;

            boost::fibers::mutex _mut;

            std::vector<std::function<void()>> _processes;

            std::vector<std::shared_ptr<par_fiber>> _fibers;

            fiber_barrier _barrier = fiber_barrier(0);

            bool _process_changed = true;

            fiber_par_internal() { }

            fiber_par_internal(std::initializer_list<std::function<void()>> &&procs)
            : _processes(std::forward<std::initializer_list<std::function<void()>>>(procs))
            {
            }

            fiber_par_internal(std::vector<std::function<void()>> &procs)
            : _processes(procs)
            {
            }

            ~fiber_par_internal()
            {
                release_all_fibers();
            }

            void release_all_fibers() noexcept
            {
                std::lock_guard<boost::fibers::mutex> lock(_mut);
                for (auto &f : _fibers)
                {
                    f->terminate();
                    f->_fiber->join();
                }
                _process_changed = true;
                _fibers.clear();
            }

            void run() noexcept
            {
                bool empty_run = true;
                std::function<void()> my_process;
                std::unique_lock<boost::fibers::mutex> lock(_mut);
                if (_processes.size() > 0)
                {
                    empty_run = false;
                    my_process = _processes[_processes.size() - 1];
                    if (_process_changed)
                    {
                        _barrier.reset(static_cast<unsigned int>(_processes.size()));
                        if (_fibers.size() < _processes.size() - 1)
                        {
                            for (unsigned int i = 0; i < _fibers.size(); ++i)
                            {
                                _fibers[i]->reset(_processes[i], _barrier);
                                _fibers[i]->release();
                            }
                            for (unsigned int i = static_cast<unsigned int>(_fibers.size()); i < _processes.size() - 1; ++i)
                            {
                                _fibers.push_back(std::shared_ptr<par_fiber>(new par_fiber(_processes[i], _barrier)));
                                _fibers[i]->start();
                            }
                        }
                        else
                        {
                            _fibers.resize(_processes.size() - 1);
                            for (unsigned int i = 0; i < _processes.size() - 1; ++i)
                            {
                                _fibers[i]->reset(_processes[i], _barrier);
                                _fibers[i]->release();
                            }
                        }
                        _process_changed = false;
                    }
                    else
                    {
                        for (unsigned int i = 0; i < _processes.size() - 1; ++i)
                            _fibers[i]->release();
                    }
                }
                if (!empty_run)
                {
                    my_process();
                    _barrier();
                }
            }

            static void add_to_all_fibers(std::shared_ptr<boost::fibers::fiber> fiber) noexcept
            {
                std::lock_guard<boost::fibers::mutex> lock(*_all_fibers_lock);
                _all_fibers.emplace(fiber);
            }

            static void remove_from_all_fibers(std::shared_ptr<boost::fibers::fiber> fiber) noexcept
            {
                std::lock_guard<boost::fibers::mutex> lock(*_all_fibers_lock);
                _all_fibers.erase(fiber);
            }
        };

        std::shared_ptr<fiber_par_internal> _internal = nullptr;

    public:
        fiber_par()
        : _internal(std::shared_ptr<fiber_par_internal>(new fiber_par_internal()))
        {
        }

        fiber_par(std::initializer_list<std::function<void()>> &&procs)
        : _internal(std::shared_ptr<fiber_par_internal>(new fiber_par_internal(std::forward<std::initializer_list<std::function<void()>>>(procs))))
        {
        }

        fiber_par(std::vector<std::function<void()>> &procs)
        : _internal(std::shared_ptr<fiber_par_internal>(new fiber_par_internal(procs)))
        {
        }

        template<typename RanIt>
        fiber_par(RanIt begin, RanIt end)
        {
            static_assert(std::iterator_traits<RanIt>::value_type == typeid(std::function<void()>), "par can only take collection of functions");
            auto procs = std::vector<std::function<void()>>(begin, end);
            _internal = std::shared_ptr<fiber_par_internal>(new fiber_par_internal(procs));
        }

        fiber_par(const fiber_par&) = default;

        fiber_par(fiber_par&&) = default;

        ~fiber_par() = default;

        fiber_par& operator=(const fiber_par&) noexcept = default;

        fiber_par& operator=(fiber_par &&) noexcept = default;

        void run() noexcept final { _internal->run(); }
    };

    thread_local std::set<std::shared_ptr<boost::fibers::fiber>> fiber_par::fiber_par_internal::_all_fibers = std::set<std::shared_ptr<boost::fibers::fiber>>();

    thread_local std::shared_ptr<boost::fibers::mutex> fiber_par::fiber_par_internal::_all_fibers_lock = std::make_shared<boost::fibers::mutex>();

    fiber_par::par_fiber::~par_fiber()
    {
        fiber_par_internal::remove_from_all_fibers(_fiber);
    }

    void fiber_par::par_fiber::run() noexcept
    {
        while (_running)
        {
            _process();
            _bar();
            _park();
        }
        fiber_par_internal::remove_from_all_fibers(_fiber);
    }

    void fiber_par::par_fiber::start() noexcept
    {
        _fiber = std::make_shared<boost::fibers::fiber>(&par_fiber::run, this);
        fiber_par_internal::add_to_all_fibers(_fiber);
        _running = true;
    }
}

#endif //CPP_CSP_PAR_H
