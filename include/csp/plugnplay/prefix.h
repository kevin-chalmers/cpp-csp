//
// Created by kevin on 26/07/16.
//

#ifndef CPP_CSP_PREFIX_H
#define CPP_CSP_PREFIX_H

#include "../process.h"
#include "../par.h"
#include "../chan.h"
#include "identity.h"

namespace csp
{
    namespace plugnplay
    {
        /*! \class prefix
         * \brief Outputs a value on the output channel then acts as identity
         *
         * \tparam T The type that the process operates on.
         *
         * \author Kevin Chalmers
         *
         * \date 26/07/2016
         */
        template<typename T>
        class prefix : public process
        {
        private:

            T _value; //<! Initial value to output

            chan_in<T> _in; //<! Input channel into the process

            chan_out<T> _out; //<! Output channel from the process

        public:
            /*!
             * \brief Creates a new prefix process.
             *
             * \param[in] value The initial value to output.
             * \param[in] in The input channel into the process.
             * \param[in] out The output channel from the process.
             */
            prefix(T value, chan_in<T> in, chan_out<T> out) noexcept
            : _value(value), _in(in), _out(out)
            {
            }

            /*!
             * \brief Executes the prefix process
             */
            void run() noexcept override final
            {
                _out(_value);
                identity<T>(_in, _out)();
            }
        };
    }
}

#endif //CPP_CSP_PREFIX_H
