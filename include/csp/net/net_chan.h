//
// Created by kevin on 07/08/16.
//

#ifndef CPP_CSP_NET_CHAN_H
#define CPP_CSP_NET_CHAN_H

#include "../chan.h"

namespace csp
{
    namespace net
    {
        struct net_address
        {
        };

        class networked
        {
        public:
            virtual net_address get_address() const noexcept = 0;
        };

        template<typename T, bool POISONABLE = false>
        class net_chan : public chan<T, POISONABLE>, public networked {
        protected:
            class net_chan_in_internal : public chan<T, POISONABLE>::chan_internal
            {
                virtual net_address get_address() const noexcept = 0;
            };
        };
    }
}

#endif //CPP_CSP_NET_CHAN_H
