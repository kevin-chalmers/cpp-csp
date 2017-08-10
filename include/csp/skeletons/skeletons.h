//
// Created by kevin on 12/08/16.
//

#ifndef CPP_CSP_SKELETONS_H
#define CPP_CSP_SKELETONS_H

#include <functional>

namespace csp
{
    namespace skeletons
    {
        template<typename T>
        class connector_in
        {
        protected:
            class connector_in_internal
            {
            public:

                virtual T read() = 0;

                virtual bool finished() const = 0;
            };

            std::shared_ptr<connector_in_internal> _internal = nullptr;

        public:

            T operator()() { return _internal->read(); }

            bool finished() const { return _internal->finished(); }
        };

        template<typename T>
        class connector_out
        {
        protected:
            class connector_out_internal
            {
            public:
                virtual void write(T value) = 0;
            };

            std::shared_ptr<connector_out_internal> _internal = nullptr;

        public:

            void operator()(T value) { _internal->write(value); }

        };

        template<typename T>
        class connector
        {
        protected:
            class connector_internal
            {

            };

            std::shared_ptr<connector_internal> _internal = nullptr;

        public:


        };

        template<typename IN, typename OUT>
        class block
        {
        protected:
            class block_internal
            {

            };
        };

        template<typename IN, typename OUT>
        class wrapper : public block<IN, OUT>
        {
        protected:
            class wrapper_internal : public block<IN, OUT>::block_internal
            {
            private:
                connector_in<IN> in;
                connector_out<OUT> out;
                std::function<OUT(IN)> fun;

                void run() noexcept
                {
                    while (!in.finished())
                    {
                        auto value = in();
                        out(fun(value));
                    }
                }
            };
        };

        template<typename IN, typename OUT, unsigned int N>
        class broadcast : public block<IN, std::array<OUT, N>>
        {

        };
    }
}

#endif //CPP_CSP_SKELETONS_H
