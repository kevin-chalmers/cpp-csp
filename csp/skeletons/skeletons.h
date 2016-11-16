//
// Created by kevin on 12/08/16.
//

#ifndef CPP_CSP_SKELETONS_H
#define CPP_CSP_SKELETONS_H

#include <functional>
#include <tuple>
#include "../process.h"
#include "../chan.h"
#include "../par.h"

namespace csp
{
    namespace skeletons
    {
        /*! \class block
         * \brief Base building block for a skeleton application.
         *
         * \author Kevin Chalmers
         *
         * \date 12/08/2016
         */
        class block : public process
        {
        protected:
            block() noexcept { }

        public:
            virtual ~block() noexcept { }
        };

        /*!
         *
         */
        template<typename IN>
        class in_block : public block
        {
        protected:
            chan_in<IN> recv = one2one_chan<IN>();

            in_block() noexcept
            : block()
            {
            }

        public:
            ~in_block() noexcept { }

            void set_recv(chan_in<IN> c) noexcept { recv = c; }

            virtual void run() noexcept = 0;
        };

        template<typename T>
        class sink : public in_block<T>
        {
        protected:
            sink() noexcept
            : in_block<T>()
            {
            }
        public:
            ~sink() noexcept { }
        };

        template<typename OUT>
        class out_block : public block
        {
        protected:
            chan_out<OUT> send = one2one_chan<OUT>();

            out_block() noexcept
            : block()
            {
            }
        public:
            ~out_block() noexcept { }

            void set_send(chan_out<OUT> c) noexcept { send = c; }

            virtual void run() noexcept = 0;
        };

        template<typename T>
        class source : public out_block<T>
        {
        protected:
            source() noexcept
            {
            }

        public:
            ~source() noexcept { }
        };

        template<typename IN, typename OUT>
        class in_out_block : public in_block<IN>, public out_block<OUT>
        {
        protected:
            in_out_block() noexcept
            : in_block<IN>(), out_block<OUT>()
            {
            }
        public:
            ~in_out_block() noexcept { }
        };

        template<typename IN, typename OUT>
        class wrapper : public in_out_block<IN, OUT>
        {
        private:
            std::function<OUT(IN)> fun;
        public:
            wrapper(std::function<OUT(IN)> fun) noexcept
            : fun(fun), in_out_block<IN, OUT>()
            {
            }

            ~wrapper() noexcept { }

            void run() noexcept override final
            {
                while (true)
                {
                    auto val = this->recv();
                    auto res = fun(val);
                    this->send(res);
                }
            }
        };

//        template<typename IN, typename OUT, size_t N>
//        class combinator_out : public block<IN, OUT>
//        {
//        protected:
//            std::array<chan_out<OUT>, N> send; //<! Outgoing array of channels from the combinator.
//        };
//
//        template<typename T, size_t N>
//        class broadcast : public combinator_out<T, T, N>
//        {
//        public:
//            void run() noexcept override final
//            {
//                while (true)
//                {
//                    auto value = this->recv();
//                    for (size_t i = 0; i < N; ++i)
//                        this->send[i](value);
//                }
//            }
//        };
//
//        template<typename T, size_t N>
//        class scatter : public combinator_out<std::array<T, N>, T, N>
//        {
//        public:
//            void run() noexcept override final
//            {
//                while (true)
//                {
//                    auto value = this->recv();
//                    for (size_t i = 0; i < N; ++i)
//                        this->send[i](value[i]);
//                }
//            }
//        };
//
//        template<typename T, size_t N>
//        class unicast_rr : public combinator_out<T, T, N>
//        {
//        public:
//            void run() noexcept override final
//            {
//                while (true)
//                {
//                    for (size_t i = 0; i < N; ++i)
//                        this->send[i](this->recv());
//                }
//            }
//        };
//
//        template<typename T, size_t N>
//        class unicast_auto : public combinator_out<T, T, N>
//        {
//        protected:
//            // Other end will be shared - auto will work automatically for this.
//            chan_out<T> send;
//        public:
//            void run() noexcept override final
//            {
//                while (true)
//                    this->send(this->recv());
//            }
//        };
//
//        template<typename IN, typename OUT, size_t N>
//        class combinator_in : public block<IN, OUT>
//        {
//        protected:
//            std::array<chan_in<IN>, N> recv;
//        };
//
//        template<typename T, size_t N>
//        class gather : public combinator_in<T, T, N>
//        {
//        public:
//            void run() noexcept override final
//            {
//                while (true)
//                {
//                    for (size_t i = 0; i < N; ++i)
//                        this->send(this->recv[i]());
//                }
//            }
//        };
//
//        template<typename T, size_t N>
//        class gatherall : public combinator_in<T, std::array<T, N>, N>
//        {
//        public:
//            void run() noexcept override final
//            {
//                while (true)
//                {
//                    std::array<T, N> value;
//                    for (size_t i = 0; i < N; ++i)
//                        value[i] = this->recv[i];
//                    this->send(value);
//                }
//            }
//        };
//
//        template<typename T>
//        class feedback : public block<T, T>
//        {
//        private:
//            class merge : public process
//            {
//            private:
//                chan_in<T> in;
//                chan_out<T> to_block;
//                chan_in<T> from_block;
//                chan_out<T> out;
//                std::function<bool(T)> cond;
//            public:
//                merge(chan_in<T> in, chan_out<T> to_block, chan_in<T> from_block, chan_out<T> out, std::function<bool(T)> cond)
//                : in(in), to_block(to_block), from_block(from_block), out(out), cond(cond)
//                {
//                }
//
//                void run() noexcept override final
//                {
//                    while (true)
//                    {
//                        to_block(in());
//                        auto value = from_block();
//                        while (cond(value))
//                        {
//                            to_block(value);
//                            value = from_block();
//                        }
//                        out(value);
//                    }
//                }
//            };
//
//            block<T, T> _block;
//
//            std::function<bool(T)> cond;
//
//        public:
//            void run() noexcept override final
//            {
//                one2one_chan<T> to_block;
//                one2one_chan<T> from_block;
//                _block.set_recv(to_block);
//                _block.set_send(from_block);
//                par
//                {
//                    _block,
//                    merge(this->recv, to_block, from_block, this->send, cond)
//                }();
//            }
//        };
//
//        template<typename IN, typename OUT, size_t N>
//        class group : public block<IN, OUT>
//        {
//        protected:
//            std::array<chan_in<IN>, N> recv; //<! The incoming array of channels into the group.
//
//            std::array<chan_out<OUT>, N> send; //<! The outgoing array of channels form the group.
//
//        public:
//        };
//
//        template<typename IN, typename OUT>
//        class indexed_wrapper : public block<IN, OUT>
//        {
//        private:
//            std::function<OUT(IN, size_t i)> fun;
//
//            size_t idx;
//        public:
//            void run() noexcept override final
//            {
//                auto f = std::bind(fun, std::placeholders::_1, idx);
//                while (true)
//                {
//                    this->send(fun(this->recv()));
//                }
//            }
//        };
//
//        template<typename IN, typename OUT, size_t N>
//        class par_n_block : public group<IN, OUT, N>
//        {
//        private:
//            std::array<indexed_wrapper<IN, OUT>, N> blocks;
//        public:
//            void run() noexcept override final
//            {
//                par(std::begin(blocks), std::end(blocks))();
//            }
//        };
//
//        template<typename IN, typename OUT, size_t N>
//        class par_block : public group<IN, OUT, N>
//        {
//        private:
//            std::array<block<IN, OUT>, N> blocks;
//        public:
//            void run() noexcept override final
//            {
//                par(std::begin(blocks), std::end(blocks))();
//            }
//        };
//
//        template<typename IN, typename OUT>
//        class pipeline : public block<IN, OUT>
//        {
//        private:
//            block<IN, OUT> _block;  //<! Constructed block form processes pipelined.
//
//
//
//        public:
//            template<typename ...args>
//            pipeline(args blocks) noexcept
//            : block<IN, OUT>()
//            {
//            }
//
//            void run() noexcept override final
//            {
//                _block();
//            }
//        };
//
//        template<typename T, size_t N, size_t k>
//        class spread : public combinator_out<T, T, N>
//        {
//        private:
//            class spreader : public process
//            {
//            private:
//                chan_in<T> recv;
//                std::array<chan_out<T>, k> send;
//                std::function<std::array<T, k>(T)> fun;
//            public:
//
//                spreader() noexcept { }
//
//                spreader(std::function<std::array<T, k>(T)> fun) noexcept
//                : fun(fun)
//                {
//                }
//
//                void set_recv(chan_in<T> c) noexcept { recv = c; }
//                void set_send(chan_out<T> c, size_t idx) noexcept { send[idx] = c; }
//
//                void run() noexcept override final
//                {
//                    while (true)
//                    {
//                        auto value = recv();
//                        auto res = fun(value);
//                        for (size_t i = 0; i < k; ++i)
//                            send[i](res[i]);
//                    }
//                }
//            };
//
//            // Internal spreader processes
//            std::array<spreader, (1 - static_cast<size_t>(pow(k, (log(N) / log(k))))) / (1 - k)> procs;
//
//        public:
//            spread(std::function<std::array<T, k>(T)> fun) noexcept
//            {
//                // Create first spreader
//                procs[0] = spreader(fun);
//                procs[0].set_recv(this->recv);
//                for (size_t level = 1; level < (log(N) / log(k)); ++level)
//                {
//                    for (size_t proc = ((1 - static_cast<size_t>(pow(k, level - 1))) / (1 - k)); proc < (1 - static_cast<size_t>(pow(k, level))) / (1 - k); ++proc)
//                    {
//                        size_t current_proc = (1 - static_cast<size_t>(pow(k, level))) / (1 - k);
//                        for (size_t output = 0; output < k; ++output)
//                        {
//                            one2one_chan<T> chan;
//                            procs[current_proc] = spreader(fun);
//                            procs[proc].set_send(chan, output);
//                            procs[current_proc].set_recv(chan);
//                            ++current_proc;
//                        }
//                    }
//                }
//                // Create last level
//                for (size_t proc = ((1 - static_cast<size_t>(pow(k, (log(N) / log(k)) - 1))) / (1 - k)), chan = 0; proc < procs.size(); ++proc)
//                {
//                    for (size_t output = 0; output < k; ++output, ++chan)
//                        procs[proc].set_send(this->send[chan], output);
//                }
//            }
//
//            void run() noexcept override final
//            {
//                par(std::begin(procs), std::end(procs))();
//            }
//        };
//
//        template<typename T, size_t N, size_t k>
//        class reduce : public combinator_in<T, T, N>
//        {
//        private:
//            class reducer : public process
//            {
//            private:
//                std::array<chan_in<T>, k> recv;
//                chan_out<T> send;
//                std::function<T(std::array<T, k>)> fun;
//            public:
//
//                reducer() noexcept { }
//
//                reducer(std::function<T(std::array<T, k>)> fun) noexcept
//                : fun(fun)
//                {
//                }
//
//                void set_recv(chan_in<T> c, size_t idx) noexcept { recv[idx] = c; }
//                void set_send(chan_out<T> c) noexcept { send = c; }
//
//                void run() noexcept override final
//                {
//                    while (true)
//                    {
//                        std::array<T, k> values;
//                        for (size_t i = 0; i < k; ++i)
//                            values[i] = recv[i]();
//                        auto value = fun(values);
//                        send(value);
//                    }
//                }
//            };
//
//            // Internal reducer processes
//            std::array<reducer, (1 - static_cast<size_t>(pow(k, (log(N) / log(k)) + 1))) / (1 - k)> procs;
//        public:
//            reduce(std::function<T(std::array<T, k>)> fun) noexcept
//            {
//                // Create last level
//                procs[0] = reducer(fun);
//                procs[0].set_send(this->send);
//                for (size_t level = 1; level < (log(N) / log(k)); ++level)
//                {
//                    for (size_t proc = ((1 - static_cast<size_t>(pow(k, level - 1))) / (1 - k)); proc < (1 - static_cast<size_t>(pow(k, level))) / (1 - k); ++proc)
//                    {
//                        size_t current_proc = (1 - static_cast<size_t>(pow(k, level))) / (1 - k);
//                        for (size_t input = 0; input < k; ++input)
//                        {
//                            one2one_chan<T> chan;
//                            procs[current_proc] = reducer(fun);
//                            procs[proc].set_recv(chan, input);
//                            procs[current_proc].set_send(chan);
//                            ++current_proc;
//                        }
//                    }
//                }
//                // Create last level
//                for (size_t proc = ((1 - static_cast<size_t>(pow(k, (log(N) / log(k)) - 1))) / (1 - k)), chan = 0; proc < procs.size(); ++proc)
//                {
//                    for (size_t input = 0; input < k; ++input, ++chan)
//                        procs[proc].set_recv(this->recv[chan], input);
//                }
//            }
//
//            void run() noexcept override final
//            {
//                par(std::begin(procs), std::end(procs))();
//            }
//        };
//
//        template<typename IN, typename OUT, typename INTERNAL>
//        block<IN, OUT> operator+(block<IN, INTERNAL> lhs, block<INTERNAL, OUT> rhs) noexcept
//        {
//            one2one_chan<INTERNAL> c;
//            lhs.set_send(c);
//            rhs.set_recv(c);
//        };
    }
}

#endif //CPP_CSP_SKELETONS_H
