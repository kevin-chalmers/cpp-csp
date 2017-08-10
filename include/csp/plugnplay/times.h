//
// Created by kevin on 26/07/16.
//

#ifndef CPP_CSP_TIMES_H
#define CPP_CSP_TIMES_H

#include "../process.h"
#include "../chan.h"
#include "../patterns.h"

namespace csp
{
    namespace plugnplay
    {
        /*! \class times
         * \brief A process that inputs two values and outputs their product.
         *
         * \tparam T The type that the process operates on.
         *
         * \author Kevin Chalmers
         *
         * \date 26/07/2016
         */
        template<typename T>
        class times : public process
        {
        private:

            chan_in<T> _in1; //<! The first input channel into the process.

            chan_in<T> _in2; //<! The second input channel into the process.

            chan_out<T> _out; //<! The output channel from the process.

        public:
            /*!
             * \brief Creates a new times process.
             *
             * \param[in] in1 The first input channel into the process.
             * \param[in] in2 The second input channel into the process.
             * \param[in] out The output channel from the process.
             */
            times(chan_in<T> in1, chan_in<T> in2, chan_out<T> out) noexcept
            : _in1(in1), _in2(in2), _out(out)
            {
            }

            /*!
             * \brief Executes the times process.
             */
            void run() noexcept override final
            {
                while (true)
                {
                    auto vals = par_read({_in1, _in2});
                    _out(vals[0] * vals[1]);
                }
            }
        };
    }
}

#endif //CPP_CSP_TIMES_H
