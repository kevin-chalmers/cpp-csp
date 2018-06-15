//
// Created by kevin on 26/07/16.
//

#ifndef CPP_CSP_TAIL_H
#define CPP_CSP_TAIL_H

#include "../process.h"
#include "../chan.h"
#include "identity.h"

namespace csp
{
    namespace plugnplay
    {
        /*! \class tail
         * \brief A process that discards its first input then behaves as identity.
         *
         * \tparam T The type that the process operates on.
         *
         * \author Kevin Chalmers
         *
         * \date 26/07/2016
         */
        template<typename T>
        class tail : public process
        {
        private:

            chan_in<T> _in; //<! The input channel into the process.

            chan_out<T> _out; //<! The output channel from the process.

        public:
            /*!
             * \brief Creates a new tail process.
             *
             * \param[in] in The input channel into the process.
             * \param[in] out The output channel from the process.
             */
            tail(chan_in<T> in, chan_out<T> out) noexcept
            : _in(in), _out(out)
            {
            }

            /*!
             * \brief Executes the tail process.
             */
            void run() noexcept override final
            {
                // Discard first value
                _in();
                // Behave as identity
                identity<T>(_in, _out)();
            }
        };
    }
}

#endif //CPP_CSP_TAIL_H
