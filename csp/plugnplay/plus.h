//
// Created by kevin on 26/06/16.
//

#ifndef CPP_CSP_PLUS_H
#define CPP_CSP_PLUS_H

#include "../process.h"
#include "../process.h"
#include "../chan.h"
#include "../patterns.h"

namespace csp
{
    namespace plugnplay
    {
        /*! \class plus
         * \brief A process that reads from its two input channels and sums upon its output.
         *
         * \tparam T The type that the process operates on.
         *
         * \author Kevin Chalmers
         *
         * \date 26/06/2016
         */
        template<typename T>
        class plus : public process
        {
        private:

            chan_in<T> _in1; //<! The first input channel into the process.

            chan_in<T> _in2; //<! The second input channel into the process.

            chan_out<T> _out; //<! The output channel from the process.

        public:
            /*!
             * \brief Creates a plus process.
             *
             * \param[in] in1 The first input channel into the process.
             * \param[in] in2 The second input channel into the process.
             * \param[in] out The output channel from the process.
             */
            plus(chan_in<T> in1, chan_in<T> in2, chan_out<T> out) noexcept
            : _in1(in1), _in2(in2), _out(out)
            {
            }

            /*!
             * \brief Executes the plus process
             */
            void run() noexcept override final
            {
                while (true)
                {
                    auto vals = par_read({_in1, _in2});
                    _out(vals[0] + vals[1]);
                }
            }
        };
    }
}

#endif //CPP_CSP_PLUS_H
