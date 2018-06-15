//
// Created by kevin on 26/06/16.
//

#ifndef CPP_CSP_PARAPLEX_H
#define CPP_CSP_PARAPLEX_H

#include <vector>
#include "../process.h"
#include "../chan.h"
#include "../patterns.h"

namespace csp
{
    namespace plugnplay
    {
        /*! \class paraplex
         * \brief A process that parallel multiplexes its inputs to its output.
         *
         * \tparam T The type that the paraplex process operates on.
         *
         * \author Kevin Chalmers
         *
         * \date 26/06/2016
         */
        template<typename T>
        class paraplex : public process
        {
        private:

            std::vector<chan_in<T>> _ins; //<! The input channels into the process.

            chan_out<std::vector<T>> _out; //<! The output channels from the process.

        public:
            /*!
             * \brief Creates a new paraplex process from a given initializer list of input channels.
             *
             * \param[in] ins The list of input channels into the process.
             * \param[in] out The output channel from the process.
             */
            paraplex(std::initializer_list<chan_in<T>> &&ins, chan_out<std::vector<T>> out) noexcept
            : _ins(ins), _out(out)
            {
            }

            /*!
             * \brief Creates a new paraplex process from a given vector of input channels.
             *
             * \param[in] ins The input channels into the process.
             * \param[in] out The output channel from the process.
             */
            paraplex(const std::vector<chan_in<T>> &ins, chan_out<std::vector<T>> out) noexcept
            : _ins(ins), _out(out)
            {
            }

            /*!
             * \brief Creates a new paraplex process from a given range of input channels.
             *
             * \tparam RanIt The type of the channel range.
             *
             * \param[in] begin The start of the channel range.
             * \param[in] end The end of the channel range.
             * \param[in] out The output channel from the process.
             */
            template<typename RanIt>
            paraplex(RanIt begin, RanIt end, chan_out<std::vector<T>> out) noexcept
            : _ins(begin, end), _out(out)
            {
                static_assert(std::iterator_traits<RanIt>::value_type == typeid(chan_in<T>), "paraplex requires a collection of chan_in");
            }

            /*!
             * \brief Executes the paraplex process.
             */
            void run() noexcept override final
            {
                while (true)
                {
                    auto values = par_read(_ins);
                    _out(values);
                }
            }
        };
    }
}

#endif //CPP_CSP_PARAPLEX_H
