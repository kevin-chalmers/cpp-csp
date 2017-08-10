//
// Created by kevin on 26/06/16.
//

#ifndef CPP_CSP_MULTIPLEX_H
#define CPP_CSP_MULTIPLEX_H

#include <vector>
#include "../process.h"
#include "../chan.h"
#include "../alt.h"
#include "multiplex_packet.h"

namespace csp
{
    namespace plugnplay
    {
        /*! \class multiplex
         * \brief A process that reads from one of its input channels and output an indexed packet on its output.
         *
         * \tparam T The type that the process operates on.
         *
         * \author Kevin Chalmers
         *
         * \date 26/06/2016
         */
        template<typename T>
        class multiplex : public process
        {
        private:

            std::vector<alting_chan_in<T>> _ins; //<! The incoming channels into the process.

            chan_out<multiplex_packet<T>> _out; //<! The outgoing channel from the process.

        public:
            /*!
             * \brief Creates a multiplex process from a given initializer list of input channels.
             *
             * \param[in] ins The incoming channels to the process.
             * \param[in] out The outgoing channel from the process.
             */
            multiplex(std::initializer_list<alting_chan_in<T>> &&ins, chan_out<multiplex_packet<T>> out) noexcept
            : _ins(ins), _out(out)
            {
            }

            /*!
             * \brief Creates a multiplex process from a given vector of input channels.
             *
             * \param[in] ins The incoming channels to the process.
             * \param[in] out The outgoing channel from the process.
             */
            multiplex(const std::vector<alting_chan_in<T>> &ins, chan_out<multiplex_packet<T>> out) noexcept
            : _ins(ins), _out(out)
            {
            }

            /*!
             * \brief Creates a multiplex process from a given range of input channels.
             *
             * \tparam RanIt The type of the channel range.
             *
             * \param[in] begin The start of the channel range.
             * \param[in] end The end of the channel range.
             * \param[in] out The output channel from the process.
             */
            template<typename RanIt>
            multiplex(RanIt begin, RanIt end, chan_out<multiplex_packet<T>> out) noexcept
            : _ins(begin, end), _out(out)
            {
                static_assert(std::iterator_traits<RanIt>::value_type == typeid(alting_chan_in<T>), "multiplex requires a collection of alting_chan_in");
            }

            /*!
             * \brief Executes the multiplex process
             */
            void run() noexcept override final
            {
                alt a(_ins.begin(), _ins.end());
                while (true)
                {
                    // Get index of ready channel.
                    auto idx = a();
                    // Read in ready value
                    auto data = _ins[idx]();
                    // Output as multiplex packet
                    _out(multiplex_packet<T>(idx, data));
                }
            }
        };
    }
}

#endif //CPP_CSP_MULTIPLEX_H
