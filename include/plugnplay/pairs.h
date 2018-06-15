//
// Created by kevin on 29/07/16.
//

#ifndef CPP_CSP_PAIRS_H
#define CPP_CSP_PAIRS_H

#include "../process.h"
#include "../par.h"
#include "../chan.h"
#include "delta.h"
#include "plus.h"
#include "tail.h"

namespace csp
{
    namespace plugnplay
    {
        /*! \class pairs
         * \brief A process that outputs the sum of successive paris of inputs.
         *
         * \tparam T The type that the process operates on.
         *
         * \author Kevin Chalmers
         *
         * \date 29/07/2016
         */
        template<typename T>
        class pairs : public process
        {
        private:

            chan_in<T> _in; //<! The input channel into the process.

            chan_out<T> _out; //<! The output channel from the process.

        public:
            /*!
             * \brief Creates a new pairs process.
             *
             * \param[in] in The input channel into the process.
             * \param[in] out The output channel from the process.
             */
            pairs(chan_in<T> in, chan_out<T> out) noexcept
            : _in(in), _out(out)
            {
            }

            /*!
             * \brief Executes the pairs process.
             */
            void run() noexcept override final
            {
                // Create network channels.
                one2one_chan<T> a;
                one2one_chan<T> b;
                one2one_chan<T> c;

                // Execute the process network
                par
                {
                    delta<T>(_in, {a, b}),
                    plus<T>(a, c, _out),
                    tail<T>(b, c)
                }();
            }
        };
    }
}

#endif //CPP_CSP_PAIRS_H
