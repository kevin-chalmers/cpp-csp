//
// Created by kevin on 03/05/16.
//

#ifndef CPP_CSP_MULTIPLEX_PACKET_H
#define CPP_CSP_MULTIPLEX_PACKET_H

namespace csp
{
    namespace plugnplay
    {
        /*! \struct multiplex_packet
         * \brief A simple structure used in multiplexing processes.
         *
         * \tparam T The type contained in the packet.
         *
         * \author Kevin Chalmers
         *
         * \date 03/05/2016
         */
        template<typename T>
        struct multiplex_packet
        {
            unsigned int index = 0;
            T data;

            /*!
             * \brief Creates a multiplex packet.
             *
             * \param[in] idx Index of the packet.
             * \param[in] dt Data in the packet.
             */
            multiplex_packet(unsigned int idx, T dt) noexcept
            : index(idx), data(dt)
            {
            }
        };
    }
}

#endif //CPP_CSP_MULTIPLEX_PACKET_H
