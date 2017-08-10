//
// Created by kevin on 12/08/16.
//

#ifndef CPP_CSP_BLOCK_H
#define CPP_CSP_BLOCK_H

#include <memory>
#include "../process.h"

namespace csp
{
    namespace skeletons
    {
        /* \class block
         * \brief Base building block for a skeleton application.
         *
         * \tparam IN The input type into the block.
         * \tparam OUT The output type from the block
         *
         * \author Kevin Chalmers
         *
         * \date 12/08/2016
         */
        template<typename IN, typename OUT>
        class block : public process
        {
        protected:
            /* \class block_internal
             * \brief Internal representation of a block.
             *
             * \author Kevin Chalmers
             *
             * \date 12/08/2016
             */
            class block_internal
            {

            };

            std::shared_ptr<block_internal> _internal = nullptr; //<! Pointer to internal representation.

        public:
            template<typename OTHER_IN, typename OTHER_OUT>
        };
    }
}

#endif //CPP_CSP_BLOCK_H
