//
// Created by kevin on 18/06/16.
//

#ifndef CPP_CSP_IDENTITY_H
#define CPP_CSP_IDENTITY_H

#include "../process.h"
#include "../chan.h"

namespace csp
{
    namespace plugnplay
    {
        /*! \class identity
         * \brief A process that outputs its input.
         *
         * \tparam T The type that the process operates on.
         *
         * \author Kevin Chalmers
         *
         * \date 18/06/2016
         */
        template<typename T>
        class identity : public process
        {
        private:

            chan_in<T> _in; //<! The input channel into the process.

            chan_out<T> _out; //<! The output channel from the process.

        public:
            /*!
             * \brief Creates a new identity process.
             *
             * \param[in] in The input channel into the process.
             * \param[in] out The output channel from the process.
             */
            identity(chan_in<T> in, chan_out<T> out) noexcept
            : _in(in), _out(out)
            {
            }

            /*!
             * \brief Executes the identity process.
             */
            void run() noexcept override final
            {
                while (true)
                    _out(_in());
            }
        };
    }
}

#endif //CPP_CSP_IDENTITY_H
