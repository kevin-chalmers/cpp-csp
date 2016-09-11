//
// Created by kevin on 26/07/16.
//

#ifndef CPP_CSP_PRINTER_H
#define CPP_CSP_PRINTER_H

#include <iostream>
#include <string>
#include "../process.h"
#include "../chan.h"

namespace csp
{
    namespace plugnplay
    {
        /*! \class printer
         * \brief A process that prints to a given output stream
         *
         * \tparam T The type that the process outputs
         *
         * \author Kevin Chalmers
         *
         * \date 26/07/2016
         */
        template<typename T>
        class printer : public process
        {
        private:

            std::reference_wrapper<std::ostream> _output; //<! The output stream used by the process.

            chan_in<T> _in; //<! The input channel into the process.

            std::string _prefix; //<! The prefix of the output.

            std::string _suffix; //<! The suffix of the output.

        public:
            /*!
             * \brief Constructs a new printer process.
             *
             * \param[in] in The input channel into the process.
             * \param[in] prefix The prefix for the output.
             * \param[in] suffix The suffix for the output.
             * \param[in] output The output stream used by the process.  Defaults to std::cout.
             */
            printer(chan_in<T> in, const std::string &prefix, const std::string &suffix, std::ostream &output = std::cout) noexcept
            : _in(in), _prefix(prefix), _suffix(suffix), _output(output)
            {
            }

            /*!
             * \brief Executes the printer process.
             */
            void run() noexcept override final
            {
                while (true)
                {
                    T _value = _in();
                    _output.get() << _prefix << _value << _suffix << std::endl;
                }
            }
        };
    }
}

#endif //CPP_CSP_PRINTER_H
