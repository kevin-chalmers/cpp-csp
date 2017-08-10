//
// Created by kevin on 28/04/16.
//

#ifndef CPP_CSP_STOP_H
#define CPP_CSP_STOP_H

#include <thread>
#include <mutex>
#include "process.h"
#include "guard.h"

namespace csp
{
    /*! \class stop
     * \brief A stop guard / process.
     *
     * \author Kevin Chalmers
     *
     * \date 28/04/2016
     */
    class stop : public guard, public process
    {
    protected:
        /*! \class stop_internal
         * \brief Internal representation of a stop.  Used for the guarded behaviour.
         *
         * \author Kevin Chalmers
         *
         * \date 28/04/2016
         */
        class stop_internal : public guard::guard_internal
        {
        protected:
            /*!
             * \brief Enables the stop process with an alt.
             *
             * \param[in] a The alt to enable the stop process with.
             *
             * \return Always false.
             */
            bool enable(const alt &a) noexcept override final
            {
                std::this_thread::yield();
                return false;
            }

            /*!
             * \brief Disables the stop process with an alt.
             *
             * \return Always false.
             */
            bool disable() noexcept override final
            {
                return false;
            }
        };

    public:
        /*!
         * \brief Creates a stop process / guard.
         */
        stop() noexcept
        : guard(std::shared_ptr<stop_internal>(new stop_internal()))
        {
        }

        /*!
         * \brief Destroys the stop process / guard.
         */
        ~stop() noexcept { }

        /*!
         * \brief Executes the stop process
         */
        void run() noexcept override final
        {
            // Create mutex
            std::mutex mut;
            // Create lock
            std::unique_lock<std::mutex> lock(mut);
            // Wait on the lock
            std::condition_variable cond;
            cond.wait(lock);
        }
    };
}

#endif //CPP_CSP_STOP_H
