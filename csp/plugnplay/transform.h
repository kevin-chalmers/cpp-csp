//
// Created by kevin on 29/07/16.
//

#ifndef CPP_CSP_TRANSFORM_H
#define CPP_CSP_TRANSFORM_H

#include <functional>
#include "../process.h"
#include "../chan.h"

namespace csp
{
    namespace plugnplay
    {
        /*! \class transform
         * \brief A process that transforms its input values by the given funtion to its output channel.
         *
         * \tparam IN The type of the input values.
         * \tparam OUT The type of the output values.
         *
         * \author Kevin Chalmers
         *
         * \date 29/07/2016
         */
        template<typename IN, typename OUT>
        class transform : public process
        {
        private:

            chan_in<IN> _in; //<! The input channel into the process.

            chan_out<OUT> _out; //<! The output channel from the process.

            std::function<OUT(IN)> _fun; //<! The function used to transform the input values.

        public:
            /*!
             * \brief Creates a new transform process
             *
             * \param[in] in The input channel into the process.
             * \param[in] out The output channel from the process.
             * \param[in] fun The function used to transform input values to the output channel.
             */
            transform(chan_in<IN> in, chan_out<OUT> out, std::function<OUT(IN)> &fun) noexcept
            : _in(in), _out(out), _fun(fun)
            {
            }

            /*!
             * \brief Executes the transform process.
             */
            void run() noexcept override final
            {
                while (true)
                    _out(_fun(_in()));
            }
        };
    }
}

#endif //CPP_CSP_TRANSFORM_H
