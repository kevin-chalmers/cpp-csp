//
// Created by kevin on 03/05/16.
//

#ifndef CPP_CSP_DELTA_H
#define CPP_CSP_DELTA_H

#include <vector>
#include "../process.h"
#include "../par.h"
#include "../patterns.h"
#include "../chan.h"

namespace csp
{
    namespace plugnplay
    {
        /*! \class delta
         * \brief Inputs a value and outputs it across its output channels.
         *
         * \tparam T The type the delta process operates on.
         * \tparam SEQUENTIAL Flag to indicate if the output should be sequential or in paralle.
         *
         * \author Kevin Chalmers
         *
         * \date 03/05/2016
         */
        template<typename T, bool SEQUENTIAL = false>
        class delta : public process
        {
        private:

            chan_in<T> _in; //<! Input channel for the process.

            std::vector<chan_out<T>> _outs; //<! Output channels for the process.

        public:
            /*!
             * \brief Constructs a new delta process with a given initializer list of output channels.
             *
             * \param[in] in The input channel into the process.
             * \param[in] outs The list of output channels from the process.
             */
            delta(chan_in<T> in, std::initializer_list<chan_out<T>> &&outs) noexcept
            : _in(in), _outs(outs)
            {
            }

            /*!
             * \brief Constructs a new delta process with a given vector of channels.
             *
             * \param[in] in The input channel into the process.
             * \param[in] outs The vector of output channels from the process.
             */
            delta(chan_in<T> in, const std::vector<chan_out<T>> &outs) noexcept
            : _in(in), _outs(outs)
            {
            }

            /*!
             * \brief Constructs a new delta process with a given range of channels.
             *
             * \tparam RanIt The type of the channel range.
             *
             * \param[in] in The input channel into the process.
             * \param[in] begin The start of the channel range.
             * \param[in] end The end of the channel range.
             */
            template<typename RanIt>
            delta(chan_in<T> in, RanIt begin, RanIt end) noexcept
            : _in(in), _outs(begin, end)
            {
                static_assert(std::iterator_traits<RanIt>::value_type == typeid(chan_out<T>), "delta requires a chan_out<T> type for output channels");
            }

            /*!
             * \brief Executes the delta process.
             */
            void run() noexcept override final
            {
                T value;
                while (true)
                {
                    value = _in();
                    if (SEQUENTIAL)
                        // Output in sequence
                        for (auto &c : _outs)
                            c(value);
                    else
                        // Output in parallel
                        par_for(_outs.begin(), _outs.end(), [&](chan_out<T> chan){ chan(value); });
                }
            }
        };
    }
}

#endif //CPP_CSP_DELTA_H
