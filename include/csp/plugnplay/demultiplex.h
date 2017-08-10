//
// Created by kevin on 03/05/16.
//

#ifndef CPP_CSP_DEMULTIPLEX_H
#define CPP_CSP_DEMULTIPLEX_H

#include <vector>
#include "../process.h"
#include "../par.h"
#include "../chan.h"
#include "multiplex_packet.h"

namespace csp
{
    namespace plugnplay
    {
        /*! \class demultiplex
         * \brief A process that reads in a multiplex packet and outputs down the appropriate channel.
         *
         * \tparam The type within the multiplex packet
         *
         * \author Kevin Chalmers
         *
         * \date 03/05/2016
         */
        template<typename T>
        class demultiplex : public process
        {
        private:

            chan_in<multiplex_packet<T>> _in; //<! Input channel into the process.

            std::vector<chan_out<T>> _outs; //<! Output channel from the process.

        public:
            /*!
             * \brief Creates a new demultiplex process from a given initializer list of channels.
             *
             * \param[in] in The input channel into the process.
             * \param[in] outs The output channels from the process.
             */
            demultiplex(chan_in<multiplex_packet<T>> in, std::initializer_list<chan_out<T>> &&outs) noexcept
            : _in(in), _outs(outs)
            {
            }

            /*!
             * \brief Creates a new demultiplex process from a given vector of output channels.
             *
             * \param[in] in The input channel into the process.
             * \param[in] outs The output channels from the process.
             */
            demultiplex(chan_in<multiplex_packet<T>> in, const std::vector<chan_out<T>> &outs) noexcept
            : _in(in), _outs(outs)
            {
            }

            /*!
             * \brief Creates a new demultiplex process from a given range of output channels.
             *
             * \tparam RanIt The type of the channel range.
             *
             * \param[in] in The input channel into the process.
             * \param[in] begin The start of the channel range.
             * \param[in] end The end of the channel range.
             */
            template<typename RanIt>
            demultiplex(chan_in<multiplex_packet<T>> in, RanIt begin, RanIt end) noexcept
            : _in(in), _outs(begin, end)
            {
                static_assert(std::iterator_traits<RanIt>::value_type == typeid(chan_out<T>), "demultiplex requires a collection of chan_out");
            }

            /*!
             * \brief Runs the demultiplex process.
             */
            void run() noexcept override final
            {
                while (true)
                {
                    auto packet = _in();
                    assert(packet.index < _outs.size());
                    _outs[packet.index](packet.data);
                }
            }
        };
    }
}

#endif //CPP_CSP_DEMULTIPLEX_H
