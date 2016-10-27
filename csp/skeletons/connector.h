//
// Created by kevin on 25/10/16.
//

#ifndef CPP_CSP_CONNECTOR_H
#define CPP_CSP_CONNECTOR_H

//
// Created by kevin on 25/10/16.
//

#include "../chan.h"
#include "block.h"

namespace csp
{
    namespace skeletons
    {
        template<typename IN>
        class connector_in
        {

        };

        template<typename OUT>
        class connector_out
        {

        };

        template<typename IN, typename OUT>
        class connector
        {
        private:
            connector_in<IN> in;
            connector_out<OUT> out;

        protected:
            class connector_internal
            {

            };

        public:
            connector_in<IN> get_input() const noexcept { return in; }
            connector_out<OUT> get_output() const noexcept { return out; }
            operator connector_in<IN>() const noexcept { return in; }
            operator connector_out<OUT>() const noexcept { return out; }
        };

        template<typename IN, typename OUT>
        class one2one_connector : public connector<IN, OUT>
        {

        };

        template<typename IN, typename OUT>
        class feedback : public one2one_connector<IN, OUT>, public block<IN, OUT>
        {

        };

        template<typename IN, typename OUT>
        class one2any_connector : public connector<IN, OUT>
        {

        };

        template<typename IN, typename OUT>
        class unicast_auto : public one2any_connector<IN, OUT>, public block<IN, OUT>
        {

        };

        template<typename IN, typename OUT>
        class any2one_connector : public connector<IN, OUT>
        {

        };

        template<typename IN, typename OUT>
        class any2any_connector : public connector<IN, OUT>
        {

        };

        template<typename IN, typename OUT>
        class many2one_connector : public connector<IN, OUT>
        {

        };

        template<typename IN, typename OUT>
        class gather : public many2one_connector<IN, OUT>, public block<IN, OUT>
        {

        };

        template<typename IN, typename OUT>
        class gatherall : public many2one_connector<IN, OUT>, public block<IN, OUT>
        {

        };

        template<typename IN, typename OUT>
        class one2many_connector : public connector<IN, OUT>
        {

        };

        template<typename IN, typename OUT>
        class broadcast : public one2many_connector<IN, OUT>, public block<IN, OUT>
        {

        };

        template<typename IN, typename OUT>
        class scatter : public one2many_connector<IN, OUT>, public block<IN, OUT>
        {

        };

        template<typename IN, typename OUT>
        class unicast_rr : public one2many_connector<IN, OUT>, public block<IN, OUT>
        {

        };
    }
}

#endif //CPP_CSP_CONNECTOR_H
