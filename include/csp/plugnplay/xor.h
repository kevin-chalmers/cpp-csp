//
// Created by kevin on 29/07/16.
//

#ifndef CPP_CSP_XOR_H
#define CPP_CSP_XOR_H

#include "../process.h"
#include "../chan.h"
#include "../patterns.h"

namespace csp
{
    namespace plugnplay
    {
        /*! \class xor_proc
         * \brief a process that performs a bitwise xor on its inputs.
         *
         * \tparam T The type that the process operates on.
         *
         * \author Kevin Chalmers
         *
         * \date 29/07/2016
         */
        template<typename T>
        class xor_proc : public process
        {
        private:

            chan_in<T> _in1; //<! The first input channel into the process.

            chan_in<T> _in2; //<! The second input channel into the process.

            chan_out<T> _out; //<! The output channel from the process.

        public:
            /*!
             * \brief Creates an xor process
             *
             * \param[in] in1 The first input channel into the process.
             * \param[in] in2 The second input channel into the process.
             * \param[in] out The output channel from the process.
             */
            xor_proc(chan_in<T> in1, chan_in<T> in2, chan_out<T> out) noexcept
            : _in1(in1), _in2(in2), _out(out)
            {
            }

            /*!
             * \brief Executes the xor process.
             */
            void run() noexcept override final
            {
                while (true)
                {
                    auto vals = par_read({_in1, _in2});
                    _out(vals[0] ^ vals[1]);
                }
            }
        };
    }
}

#endif //CPP_CSP_XOR_H
