//
// Created by kevin on 18/06/16.
//

#ifndef CPP_CSP_FIXED_DELAY_H
#define CPP_CSP_FIXED_DELAY_H

#include "../process.h"
#include "../chan.h"
#include "../timer.h"

namespace csp
{
    namespace plugnplay
    {
        /*! \class fixed_delay
         * \brief A Process that copies the input to the output, imposing a
         * fixed delay between exents.
         *
         * \tparam T The type that the process operates on.
         *
         * \author Kevin Chalmers
         *
         * \date 18/06/2016
         *
         */
        template<typename T>
        class fixed_delay : public process
        {
        private:

            chan_in<T> _in; //<! The input channel.

            chan_out<T> _out; //<! The output channel.

            std::chrono::steady_clock::duration _delay; //<! The delay time.

        public:
            /*!
             * \brief Creates a new fixed delay process.
             *
             * \param[in] delay The delay imposed by the process.
             * \param[in] in The input channel into the process.
             * \param[in] out The output channel from the process.
             */
            fixed_delay(const std::chrono::steady_clock::duration delay, chan_in<T> in, chan_out<T> out) noexcept
            : _delay(delay), _in(in), _out(out)
            {
            }

            /*!
             * \brief Executes the fixed delay process.
             */
            void run() noexcept override final
            {
                timer t;
                while (true)
                {
                    T v = _in();
                    t(_delay);
                    _out(v);
                }
            }
        };
    }
}

#endif //CPP_CSP_FIXED_DELAY_H
