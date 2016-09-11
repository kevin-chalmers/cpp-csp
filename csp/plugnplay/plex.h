//
// Created by kevin on 26/06/16.
//

#ifndef CPP_CSP_PLEX_H
#define CPP_CSP_PLEX_H

#include <vector>
#include "../process.h"
#include "../alt.h"
#include "../chan.h"

namespace csp
{
    namespace plugnplay
    {
        /*! \class plex
         * \brief A process that fairly multiplexes its input upon its output.
         *
         * \tparam T The type that the process operates on.
         *
         * \author Kevin Chalmers
         *
         * \date 26/06/2016
         */
        template<typename T>
        class plex : public process
        {
        private:

            std::vector<alting_chan_in<T>> _ins; //<! The input channels into the process.

            chan_out<T> _out; //<! The output channel from the process.

        public:
            /*!
             * \brief Creates a plex process from a given list of channels.
             *
             * \param[in] ins The input channels into the process.
             * \param[in] out The output channel from the process.
             */
            plex(std::initializer_list<alting_chan_in<T>> &&ins, chan_out<T> out) noexcept
            : _ins(ins), _out(out)
            {
            }

            /*!
             * \brief Creates a plex process from a given vector of channels.
             *
             * \param[in] ins The input channels into the process.
             * \param[in] out The output channel from the process.
             */
            plex(const std::vector<alting_chan_in<T>> &ins, chan_out<T> out) noexcept
            : _ins(ins), _out(out)
            {
            }

            /*!
             * \brief Creates a plex process from a given range of input channels.
             *
             * \tparam RanIt The type of the channel range.
             *
             * \param[in] begin The start of the channel range.
             * \param[in] end The end of the channel range.
             * \param[in] out The output channel from the process.
             */
            template<typename RanIt>
            plex(RanIt begin, RanIt end, chan_out<T> out) noexcept
            : _ins(begin, end), _out(out)
            {
            }

            /*!
             * \brief Executes the plex process.
             */
            void run() noexcept override final
            {
                // Create alt from input channels.
                alt a(_ins.begin(), _ins.end());
                while (true)
                    _out(_ins[a()]());
            }
        };
    }
}

#endif //CPP_CSP_PLEX_H
