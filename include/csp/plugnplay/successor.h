//
// Created by kevin on 26/07/16.
//

#ifndef CPP_CSP_SUCCESSOR_H
#define CPP_CSP_SUCCESSOR_H

#include "../process.h"
#include "../chan.h"

namespace csp
{
    namespace plugnplay
    {
        /*! \class successor
         * \brief A process that increments its input.
         *
         * \tparam T The type that the process operates on.
         *
         * \author Kevin Chalmers
         *
         * \date 26/07/2016
         */
        template<typename T>
        class successor : public process
        {
        private:

            chan_in<T> _in; //<! The input channel into the process.

            chan_out<T> _out; //<! The output channel from the process.

        public:
            /*!
             * \brief Creates a new successor process.
             *
             * \param[in] in The input channel into the process.
             * \param[in] out The output channel from the process.
             */
            successor(chan_in<T> in, chan_out<T> out) noexcept
            : _in(in), _out(out)
            {
            }

            /*!
             * \brief Executes the successor process.
             */
            void run() noexcept override final
            {
                while (true)
                {
                    auto val = _in();
                    _out(++val);
                }
            }
        };
    }
}

#endif //CPP_CSP_SUCCESSOR_H
