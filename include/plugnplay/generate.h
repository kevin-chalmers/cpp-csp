//
// Created by kevin on 18/06/16.
//

#ifndef CPP_CSP_GENERATE_H
#define CPP_CSP_GENERATE_H

#include "../process.h"
#include "../chan.h"

namespace csp
{
    namespace plugnplay
    {
        /*! \class generate
         * \brief A process that constantly outputs the value provided.
         *
         * \tparam T The type of the value that the process generates.
         *
         * \author Kevin Chalmers
         *
         * \date 18/06/2016
         */
        template<typename T>
        class generate : public process
        {
        private:

            chan_out<T> _out; //<! The output channel.

            T _value; //<! The value to output.

        public:
            /*!
             * \brief Creates a generate process.
             *
             * \param[in] out The output channel from the process.
             * \param[in] value The value output by the process.
             */
            generate(chan_out<T> out, T value) noexcept
            : _out(out), _value(value)
            {
            }

            /*!
             * \brief Executes the generate process.
             */
            void run() noexcept override final
            {
                while (true)
                    _out(_value);
            }
        };
    }
}

#endif //CPP_CSP_GENERATE_H
