//
// Created by kevin on 02/05/16.
//

#ifndef CPP_CSP_AND_H
#define CPP_CSP_AND_H

#include "../process.h"
#include "../par.h"
#include "../chan.h"

namespace csp
{
    namespace plugnplay
    {
        /*! \class and_proc
         * \brief Process that performs a bitwise and on its inputs
         *
         * \tparam T The type that the process operates on.
         *
         * \author Kevin Chalmers
         *
         * \date 02/05/2016
         */
        template<typename T>
        class and_proc : public process
        {
        private:

            chan_in<T> _in0; //<! Input channel 0

            chan_in<T> _in1; //<! Input channel 1

            chan_out<T> _out; //<! Output channel

        public:
            /*!
             * \brief Constructs the and process.
             *
             * \param[in] in0 Input channel 0.
             * \param[in] in1 Input channel 1.
             * \param[in] out Output channel.  Output is in0 | in1
             */
            and_proc(chan_in<T> in0, chan_in<T> in1, chan_out<T> out) noexcept
            : _in0(in1), _in1(in1), _out(out)
            {
                static_assert(std::is_integral<T>::value, "Integral required");
            }

            /*!
             * \brief Runs the process.
             */
            void run() noexcept override final
            {
                while (true)
                {
                    auto vals = par_read({_in0, _in1});
                    _out(vals[0] & vals[1]);
                }
            }
        };
    }
}

#endif //CPP_CSP_AND_H
