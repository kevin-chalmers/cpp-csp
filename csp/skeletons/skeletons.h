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
        template<typename IN_POLICY, typename OUT_POLICY>
        class block : private IN_POLICY, private OUT_POLICY, private process
        {
        protected:
            class block_internal : public IN_POLICY::internal, public OUT_POLICY::internal
            {
            public:
                virtual ~block_internal() noexcept { }
                virtual void run() noexcept = 0;
            };

            std::shared_ptr<block_internal> _internal = nullptr;

            block(std::shared_ptr<block_internal> internal) noexcept
            : IN_POLICY(), OUT_POLICY(), _internal(internal)
            {
            }
        public:
            virtual ~block() noexcept { }

            void run() noexcept override final { _internal->run(); }
        };

        struct empty_in
        {
        protected:
            class internal
            {
            };

            ~empty_in() noexcept { }
        };

        struct empty_out
        {
        protected:
            class internal
            {
            };

            ~empty_out() noexcept { }
        };

        template<typename T>
        struct single_in
        {
        protected:
            class internal
            {
            public:
                chan_in<T> recv = one2one_chan<T>();
            };

            std::shared_ptr<single_in> _internal = nullptr;

            ~single_in() noexcept { }
        public:
            void set_recv(chan_in<T> c) noexcept { _internal->recv = c; }
        };

        template<typename T>
        struct single_out
        {
        protected:
            class internal
            {
            public:
                chan_out<T> send = one2one_chan<T>();
            };

            std::shared_ptr<single_out> _internal = nullptr;

            ~single_out() noexcept { }
        public:
            void set_send(chan_out<T> c) noexcept { _internal->send = c; }
        };

        struct empty_run
        {
        protected:
            class internal
            {
            };

            ~empty_run() { }
        };

        template<typename T>
        class sink : public block<single_in<T>, empty_out>
        {
        protected:
            class sink_internal : public block<single_in<T>, empty_out>::block_internal
            {
                using single_in<T>::recv;
            private:
                std::function<void(chan_in<T>)> fun;
            public:
                sink_internal(std::function<void(chan_in<T>)> fun) noexcept
                : fun(fun)
                {
                }

                void run() noexcept override final
                {
                    fun(recv);
                }
            };
        public:
            sink(std::function<void(chan_in<T>)> fun) noexcept
            : block<single_in<T>, empty_out>(std::shared_ptr<sink_internal>(new sink_internal(fun)))
            {
            }

            ~sink() noexcept { }
        };

        template<typename T>
        class source : public block<empty_in, single_out<T>>
        {
        protected:
            class source_internal : public block<empty_in, single_out<T>>::block_internal
            {
            private:
                std::function<void(chan_out<T>)> fun;
                std::reference_wrapper<chan_out<T>> chan;
            public:
                source_internal(std::function<void(chan_out<T>)> fun, chan_out<T> &chan) noexcept
                : fun(fun), chan(chan)
                {
                }

                void run() noexcept override final
                {
                    fun(chan);
                }
            };
        public:
            source(std::function<void(chan_out<T>)> fun) noexcept
            : block<empty_in, single_out<T>>(std::shared_ptr<source_internal>(new source_internal(fun, this->send)))
            {
            }

            ~source() noexcept { }
        };

        template<typename IN, typename INTERNAL_OUT, typename INTERNAL_IN, typename OUT>
        class union_block : public block<IN, OUT>
        {
        protected:
            class union_block_internal : public block<IN, OUT>::block_internal
            {
            private:
                block<IN, INTERNAL_OUT> lhs_block;
                block<INTERNAL_IN, OUT> rhs_block;
            public:
                union_block_internal(block<IN, INTERNAL_OUT> lhs_block, block<INTERNAL_IN, OUT> rhs_block) noexcept
                : lhs_block(lhs_block), rhs_block(rhs_block)
                {
                }

                void run() noexcept override final
                {
                    par
                    {
                        lhs_block,
                        rhs_block
                    }();
                }
            };

        public:
            union_block(block<IN, INTERNAL_OUT> &lhs, block<INTERNAL_IN, OUT> &rhs) noexcept
            : block<IN, OUT>(std::shared_ptr<union_block_internal>(new union_block_internal(lhs, rhs)))
            {
            }
        };

        template<typename LHS_TYPE, typename INTERNAL_TYPE_OUT, typename INTERNAL_TYPE_IN, typename RHS_TYPE>
        block<LHS_TYPE, RHS_TYPE> operator+(block<LHS_TYPE, INTERNAL_TYPE_OUT> lhs, block<INTERNAL_TYPE_IN, RHS_TYPE> rhs) noexcept;

        template<typename T>
        block<empty_in, empty_out> operator+(source<T> lhs, sink<T> rhs) noexcept
        {
            one2one_chan<T> c;
            lhs.set_send(c);
            rhs.set_recv(c);
            return union_block<empty_in, single_out<T>, single_in<T>, empty_out>(lhs, rhs);
        }

        /*template<typename IN, typename OUT>
        class wrapper : public block<single_in<IN>, single_out<OUT>, runnable>
        {
        private:
            std::function<OUT(IN)> fun;
        public:
            wrapper(std::function<OUT(IN)> fun) noexcept
            : fun(fun), block<single_in<IN>, single_out<OUT>, runnable>()
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



        */

        /*template<typename IN, typename OUT>
        class pipeline : public block<IN, OUT>
        {
        private:
            std::vector<block

        public:
            template<typename ...args>
            pipeline(args blocks) noexcept
            : block<IN, OUT>()
            {
            }

            void run() noexcept override final
            {
                _block();
            }
        };*/


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
//
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
