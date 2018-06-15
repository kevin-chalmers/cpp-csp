//
// Created by kevin on 26/06/16.
//

#ifndef CPP_CSP_MULT_H
#define CPP_CSP_MULT_H

#include "../process.h"
#include "../chan.h"

namespace csp
{
    namespace plugnplay
    {
        /*! \class mult
         * \brief A process that multiplies its input by some value and then outputs.
         *
         * \tparam T The type that the process operates on.
         *
         * \author Kevin Chalmers
         * \date 26/06/16
         */
        template<typename T>
        class mult : public process
        {
        private:

            const T _n; //<! The multiplier.

            chan_in<T> _in; //<! The input channel into the process.

            chan_out<T> _out; //<! The output channel from the process.

        public:
            /*!
             * \brief Creates a new mult process.
             *
             * \param[in] n Value to multiply incoming values by.
             * \param[in] in The input channel into the process.
             * \param[in] out The output channel from the process.
             */
            mult(T n, chan_in<T> in, chan_out<T> out) noexcept
            : _n(n), _in(in), _out(out)
            {
            }

            /*!
             * \brief Executes the mult process.
             */
            void run() noexcept override final
            {
                while (true)
                    _out(_n * _in());
            }
        };
    }
}

#endif //CPP_CSP_MULT_H
