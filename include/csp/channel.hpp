//
// Created by kevin on 08/03/19.
//

#ifndef CPP_CSP_CHANNEL_H
#define CPP_CSP_CHANNEL_H

#include <memory>

namespace csp
{
    template<typename T>
    class channel_internal
    {
    };

    template<typename T>
    class channel
    {
    private:
        std::shared_ptr<channel_internal<T>> _internal = nullptr;
    };

    template<typename T>
    class chan_in
    {
    };

    template<typename T>
    class guarded_chan_in : public chan_in<T>
    {
    };

    template<typename T>
    class shared_chan_in : public chan_in<T>
    {
    };

    template<typename T>
    class chan_out
    {
    };

    template<typename T>
    class shared_chan_out : public chan_out<T>
    {
    };

    template<typename T, template<typename> typename INPUT_END, template<typename> typename OUTPUT_END>
    class chan_type
    {
    private:
        INPUT_END<T> _input;
        OUTPUT_END<T> _output;
        channel<T> _chan;
    public:
        inline INPUT_END<T> in() const { return _input; }
        inline OUTPUT_END<T> out() const { return _output; }
    };

    template<typename T>
    using one2one_chan = chan_type<T, guarded_chan_in, chan_out>;

    template<typename T>
    using one2any_chan = chan_type<T, shared_chan_in, chan_out>;

    template<typename T>
    using any2one_chan = chan_type<T, guarded_chan_in, shared_chan_out>;

    template<typename T>
    using any2any_chan = chan_type<T, shared_chan_in, shared_chan_out>;
}

#endif //CPP_CSP_CHANNEL_H
