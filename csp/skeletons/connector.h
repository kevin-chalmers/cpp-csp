//
// Created by kevin on 11/09/16.
//

#ifndef CPP_CSP_CONNECTOR_H
#define CPP_CSP_CONNECTOR_H

#include <memory>
#include "../chan.h"

namespace csp
{
    namespace skeletons
    {
        template<typename IN, typename OUT>
        class connector
        {
        private:
            class connector_internal
            {
            public:
                virtual void write(IN value) noexcept(false) = 0;
                virtual OUT read() noexcept(false) = 0;
                virtual ~connector_internal() noexcept { }
            };

            std::shared_ptr<connector_internal> _internal = nullptr;
        protected:
            connector() { }
        public:
            connector(const connector &other) noexcept = default;
            connector(connector &&rhs) noexcept = default;
            connector& operator=(const connector &other) noexcept = default;
            connector& operator=(connector &&rhs) noexcept = default;
            virtual ~connector() noexcept { }
            void write(IN value) const noexcept(false) { _internal->write(value); }
            OUT read() const noexcept(false) { return _internal->read(); }
        };
    }
}

#endif //CPP_CSP_CONNECTOR_H
