//
// Created by kevin on 26/07/16.
//

#ifndef CPP_CSP_SUBSTITUTE_H
#define CPP_CSP_SUBSTITUTE_H

#include "../process.h"
#include "../chan.h"

namespace csp
{
    namespace plugnplay
    {
        /*! \class substitute
         * \brief A process that substitutes its input for a given value.
         *
         * \tparam T The type that the process operates on.
         *
         * \author Kevin Chalmers
         *
         * \date 26/07/2016
         */
        template<typename T>
        class substitute : public process
        {
        private:

            chan_in<T> _in; //<! The input channel into the process.

            chan_out<T> _out; //<! The output channel from the process.

            T _value; //<! The value to be sent down the output channel.

        public:
            /*!
             * \brief Creates a substitute process.
             *
             * \param[in] in The input channel into the process.
             * \param[in] out The output channel from the process.
             * \param[in] value The value to be sent down the output channel.
             */
            substitute(chan_in<T> in, chan_out<T> out, T value) noexcept
            : _in(in), _out(out), _value(value)
            {
            }

            /*!
             * \brief Executes the substitute process.
             */
            void run() noexcept override final
            {
                while (true)
                {
                    _in();
                    _out(_value);
                }
            }
        };
    }
}

#endif //CPP_CSP_SUBSTITUTE_H
