//
// Created by kevin on 26/07/16.
//

#ifndef CPP_CSP_REGULATE_H
#define CPP_CSP_REGULATE_H

#include <vector>
#include "../process.h"
#include "../alt.h"
#include "../chan.h"
#include "../timer.h"

namespace csp
{
    namespace plugnplay
    {
        /*! \class regulate
         * \brief A process that controls the flow of input to its output.
         *
         * \tparam T The type that the process operates on.
         *
         * \author Kevin Chalmers
         *
         * \date 26/07/2016
         */
        template<typename T>
        class regulate : public process
        {
        private:

            alting_chan_in<T> _in; //<! The input channel into the process.

            alting_chan_in<std::chrono::steady_clock::duration> _reset; //<! The channel used to change the interval between outputs.

            chan_out<T> _out; //<! The output channel.

            std::chrono::steady_clock::duration _interval; //<! The interval between outputs.

        public:
            /*!
             * \brief Creates a new regulate process.
             *
             * \param[in] in The input channel into the process.
             * \param[in] reset The channel used to change the interval between outputs.
             * \param[in] out The output channel from the process.
             * \param[in] interval The interval between outputs.
             */
            regulate(alting_chan_in<T> in, alting_chan_in<std::chrono::steady_clock::duration> reset, chan_out<T> out, std::chrono::steady_clock::duration interval) noexcept
            : _in(in), _reset(reset), _out(out), _interval(interval)
            {
            }

            /*!
             * \brief Executes the regulate process
             */
            void run() noexcept override final
            {
                constexpr int RESET = 0;
                constexpr int TIME = 1;
                constexpr int IN = 2;

                timer t;
                alt a{_reset, t, _in};
                t += _interval;
                T value;
                std::vector<bool> pre_cond{true, false, true};

                while (true)
                {
                    switch (a.pri_select(pre_cond))
                    {
                        case RESET:
                            _interval = _reset();
                            break;
                        case TIME:
                            _out(value);
                            t += _interval;
                            pre_cond[1] = false;
                            pre_cond[2] = true;
                            break;
                        case IN:
                            value = _in();
                            pre_cond[1] = true;
                            pre_cond[2] = false;
                            break;
                    }
                }
            }
        };
    }
}

#endif //CPP_CSP_REGULATE_H
