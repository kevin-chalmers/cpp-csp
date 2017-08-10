//
// Created by kevin on 26/07/16.
//

#ifndef CPP_CSP_SIGN_H
#define CPP_CSP_SIGN_H

#include <string>
#include <sstream>
#include "../process.h"
#include "../chan.h"

namespace csp
{
    namespace plugnplay
    {
        /*! \class sign
         * \brief A process that inputs a value and outputs it as a string with a given prefix.
         *
         * \tparam T The type that the process operates on.
         *
         * \author Kevin Chalmers
         *
         * \date 26/07/2016
         */
        template<typename T>
        class sign : public process
        {
        private:

            chan_in<T> _in; //<! The input channel into the process.

            chan_out<std::string> _out; //<! The output channel from the process.

            std::string _sign; //<! The sign (prefix) to attach to each item.

        public:
            /*!
             * \brief Creates a new sign process.
             *
             * \param[in] in The input channel into the process.
             * \param[in] out The output channel from the process.
             * \param[in] sign The sign (prefix) to attach to each item.
             */
            sign(chan_in<T> in, chan_out<std::string> out, const std::string &sign) noexcept
            : _in(in), _out(out), _sign(sign)
            {
            }

            /*!
             * \brief Executes the sign process.
             */
            void run() noexcept override final
            {
                while (true)
                {
                    std::stringstream stream;
                    stream << _sign << _in();
                    _out(stream.str());
                }
            }
        };
    }
}

#endif //CPP_CSP_SIGN_H
