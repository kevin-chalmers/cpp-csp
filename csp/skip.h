//
// Created by kevin on 28/04/16.
//

#ifndef CPP_CSP_SKIP_H
#define CPP_CSP_SKIP_H

#include <thread>
#include "process.h"
#include "guard.h"

namespace csp
{
    /*! \class skip
     * \brief A skip guard / process.
     *
     * \author Kevin Chalmers
     *
     * \date 28/04/2016
     */
    class skip : public process, public guard
    {
    protected:
        /*! \class skip_internal
         *
         * \brief Internal representation of the skip.  Used to allow guarded behaviour.
         *
         * \author Kevin Chalmers
         *
         * \date 28/04/2016
         */
        class skip_internal : public guard::guard_internal
        {
        protected:
            /*!
             * \brief Enables the skip with an alt.
             *
             * \param[in] a The alt to enable the skip with.
             *
             * \return Always true.
             */
            bool enable(const alt &a) noexcept override final
            {
                std::this_thread::yield();
                return false;
            }

            /*!
             * \brief Disables the skip with an alt.
             *
             * \return Always true.
             */
            bool disable() noexcept override final
            {
                return false;
            }
        };

    public:
        /*!
         * \brief Creates a skip process / guard.
         */
        skip() noexcept
        : guard(std::shared_ptr<skip_internal>(new skip_internal()))
        {
        }

        /*!
         * \brief Destroys the skip process / guard.
         */
        ~skip() noexcept { }

        /*!
         * \brief Executes the skip process.
         */
        void run() noexcept override final { }
    };
}

#endif //CPP_CSP_SKIP_H
