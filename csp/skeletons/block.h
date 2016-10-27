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
            public:

                virtual void run() noexcept = 0;
            };

            std::shared_ptr<block_internal> _internal = nullptr; //<! Pointer to internal representation.

        public:

        };

        template<typename OUT>
        class source : public block<void, OUT>
        {
        protected:
            class source_internal : public block<void, OUT>::block_internal
            {


            public:

            };
        };

        template<typename IN>
        class sink : public block<IN, void>
        {

        };

        template<typename IN, typename OUT>
        class wrapper : public block<IN, OUT>
        {
        protected:
            class wrapper_internal : block<IN, OUT>::block_internal
            {
            public:

                std::function<IN, OUT> _fun;
            };
        };

        template<typename IN, typename OUT>
        class functional : public block<IN, OUT>
        {

        };

        template<typename IN, typename OUT>
        class par_block : public functional<IN, OUT>
        {

        };

        template<typename IN, typename OUT>
        class par_n_block : public functional<IN, OUT>
        {

        };

        template<typename IN, typename OUT>
        class pipeline : public functional<IN, OUT>
        {

        };

        template<typename IN, typename OUT>
        class spread : public functional<IN, OUT>
        {

        };

        template<typename IN, typename OUT>
        class reduce : public functional<IN, OUT>
        {

        };
    }
}

#endif //CPP_CSP_BLOCK_H
