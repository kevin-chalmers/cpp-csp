//
// Created by kevin on 29/07/16.
//

#ifndef CPP_CSP_NUMBERS_H
#define CPP_CSP_NUMBERS_H

#include "../process.h"
#include "../chan.h"
#include "delta.h"
#include "successor.h"
#include "prefix.h"

namespace csp
{
    namespace plugnplay
    {
        /*! \class numbers
         * \brief A process that outputs the natural numbers.
         *
         * \tparam T The type that the process outputs.  Needs to be unsigned intergral.
         *
         * \author Kevin Chalmers
         *
         * \date 29/07/2016
         */
        template<typename T>
        class numbers : public process
        {
        private:

            chan_out<T> _out; //<! The output channel from the process.

        public:
            /*!
             * \brief Creates a new numbers process.
             */
            numbers(chan_out<T> out) noexcept
            : _out(out)
            {
            }

            /*!
             * \brief Executes the numbers process.
             */
            void run() noexcept override final
            {
                // Create network channels.
                one2one_chan<T> a;
                one2one_chan<T> b;
                one2one_chan<T> c;

                par
                {
                    delta<T>(a, {b, _out}),
                    successor<T>(b, c),
                    prefix<T>(0, c, a)
                }();
            }
        };
    }
}

#endif //CPP_CSP_NUMBERS_H
