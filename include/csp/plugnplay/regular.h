//
// Created by kevin on 26/07/16.
//

#ifndef CPP_CSP_REGULAR_H
#define CPP_CSP_REGULAR_H

#include "../process.h"
#include "../chan.h"
#include "../timer.h"

namespace csp
{
    namespace plugnplay
    {
        /*! \class regular
         * \brief A process that outputs a given value at regular intervals.
         *
         * \tparam T The type that the process operates on.
         *
         * \author Kevin Chalmers
         *
         * \date 26/07/2016
         */
        template<typename T>
        class regular : public process
        {
        private:

            chan_out<T> _out; //<! The output channel from the process.

            T _value; //<! The value to output from the process.

            std::chrono::steady_clock::duration _interval; //<! The interval between outputs.

        public:
            /*!
             * \brief Creates a new regular process.
             *
             * \param[in] out The output channel from the process.
             * \param[in] value The value to output from the process.
             * \param[in] duration The interval between outputs.
             */
            regular(chan_out<T> out, T value, std::chrono::steady_clock::duration interval) noexcept
            : _out(out), _value(value), _interval(interval)
            {
            }

            /*!
             * \brief Executes the regular process.
             */
            void run() noexcept override final
            {
                timer t;
                auto next = t();
                while (true)
                {
                    _out(_value);
                    next += _interval;
                    t(next);
                }
            }
        };
    }
}

#endif //CPP_CSP_REGULAR_H
