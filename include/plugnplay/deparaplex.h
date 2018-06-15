//
// Created by kevin on 05/05/16.
//

#ifndef CPP_CSP_DEPARAPLEX_H
#define CPP_CSP_DEPARAPLEX_H

#include <vector>
#include "../process.h"
#include "../patterns.h"
#include "../chan.h"

namespace csp
{
    // TODO: should these processes use array type and have fixed size?

    namespace plugnplay
    {
        /*! \class deparaplex
         * \brief A process that takes in an array of values and outputs them on its output channels.
         *
         * \tparam T The type that the channels operate on.
         *
         * \author Kevin Chalmers
         *
         * \date 05/05/2016
         */
        template<typename T>
        class deparaplex : public process
        {
        private:

            chan_in<std::vector<T>> _in; //<! Input channel of vector type.

            std::vector<chan_out<T>> _outs; //<! Output channels that vector is split across.

        public:
            /*!
             * \brief Creates a new deparaplex process from a given input channel and list of output channels
             *
             * \param[in] in The input channel into the process.
             * \param[in] outs The output channels from the process.
             */
            deparaplex(chan_in<std::vector<T>> in, std::initializer_list<chan_out<T>> &&outs) noexcept
            : _in(in), _outs(outs)
            {
            }

            /*!
             * \brief Creates a new deparaplex process from a given input channel and a vector of output channels.
             *
             * \param[in] in The input channel into the process.
             * \param[in] outs The output channels from the process.
             */
            deparaplex(chan_in<std::vector<T>> in, const std::vector<chan_out<T>> &outs) noexcept
            : _in(in), _outs(outs)
            {
            }

            /*!
             * \brief Creates a new deparaplex process from an input channel and a given range of output channels.
             *
             * \tparam RanIt The type of the output channel range.
             *
             * \param[in] in The input channel into the process.
             * \param[in] begin The start of the output channel range.
             * \param[in] end The end of the output channel range.
             */
            template<typename RanIt>
            deparaplex(chan_in<std::vector<T>> in, RanIt begin, RanIt end) noexcept
            : _in(in), _outs(begin, end)
            {
                static_assert(std::iterator_traits<RanIt>::value_type == typeid(chan_out<T>), "deparaplex requires a collection of chan_out");
            }

            /*!
             * \brief Executes the deparaplex process.
             */
            void run() noexcept override final
            {
                while (true)
                {
                    auto data = _in();
                    assert(data.size() == _outs.size());
                    par_write(_outs, data);
                }
            }
        };
    }
}

#endif //CPP_CSP_DEPARAPLEX_H
