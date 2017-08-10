//
// Created by kevin on 06/06/16.
//

#ifndef CPP_CSP_DYNAMIC_DELTA_H
#define CPP_CSP_DYNAMIC_DELTA_H

#include <algorithm>
#include <vector>
#include "../process.h"
#include "../par.h"
#include "../patterns.h"
#include "../chan.h"

namespace csp
{
    namespace plugnplay
    {
        /*! \class dynamic_delta
         * \brief A delta process which can add or remove channels.
         *
         * \tparam T The type that the process operates with.
         * \tparam SEQUENTIAL Flag to indicate whether the process should run sequentially
         *
         * \author Kevin Chalmers
         *
         * \date 06/06/2016
         *
         */
        template<typename T, bool SEQUENTIAL=false>
        class dynamic_delta : public process
        {
        private:

            alting_chan_in<T> _in; //<! The input channel into the process.

            alting_chan_in<chan_out<T>> _config; //<! The configuration channel.

            std::vector<chan_out<T>> _outs; //<! The output channels from the process.

        public:
            /*!
             * \brief Creates a dynamic delta process with no initial output channels
             *
             * \param[in] in The input channel into the process.
             * \param[in] config The configuration channel for the process.
             */
            dynamic_delta(alting_chan_in<T> in, alting_chan_in<chan_out<T>> config) noexcept
            : _in(in), _config(config)
            {
            }

            /*!
             * \brief Creates a dynamic delta process with a given initializer list of output channels
             *
             * \param[in] in The input channel into the process.
             * \param[in] config The configuration channel for the process.
             * \param[in] outs The output channels from the process.
             */
            dynamic_delta(alting_chan_in<T> in, alting_chan_in<chan_out<T>> config, std::initializer_list<chan_out<T>> &&outs) noexcept
            : _in(in), _config(config), _outs(outs)
            {
            }

            /*!
             * \brief Creates a dynamic delta process with a given vector of output channels.
             *
             * \param[in] in The input channel into the process.
             * \param[in] config The configuration channel for the process.
             * \param[in] outs The output channels from the process.
             */
            dynamic_delta(alting_chan_in<T> in, alting_chan_in<chan_out<T>> config, const std::vector<chan_out<T>> &outs) noexcept
            : _in(in), _config(config), _outs(outs)
            {
            }

            /*!
             * \brief Creates a dynamic delta process with a given range of output channels.
             *
             * \param[in] in The input channel into the process.
             * \param[in] config The configuration channel for the process.
             * \param[in] outs The output channels from the process.
             *
             * \tparam RanIt The range type used to create the output channel.
             */
            template<typename RanIt>
            dynamic_delta(alting_chan_in<T> in, alting_chan_in<chan_out<T>> config, RanIt begin, RanIt end) noexcept
            : _in(in), _config(config), _outs(begin, end)
            {
                static_assert(std::iterator_traits<RanIt>::value_type == typeid(chan_out<T>), "dynamic delta requires a collection of chan_out");
            }

            /*!
             * \brief Executes the dynamic delta process
             */
            void run() noexcept override final
            {
                alt a{ _config, _in };
                while (true)
                {
                    // Select ready channel
                    switch (a())
                    {
                        // Configuration channel
                        case 0:
                        {
                            // Read in channel
                            auto c = _config();
                            // Check if channel exists in vector
                            auto found = std::find(_outs.begin(), _outs.end(), c);
                            if (found != _outs.end())
                                _outs.erase(found);
                            else
                                _outs.push_back(c);
                            break;
                        }

                        // Input channel
                        case 1:
                        {
                            auto value = _in();
                            if (SEQUENTIAL)
                                // Output in sequence
                                for (auto &c : _outs)
                                    c(value);
                            else
                                // Output in parallel
                                par_for(_outs.begin(), _outs.end(), [&](chan_out<T> chan){ chan(value); });
                            break;
                        }
                    }
                }
            }
        };
    }
}

#endif //CPP_CSP_DYNAMIC_DELTA_H
