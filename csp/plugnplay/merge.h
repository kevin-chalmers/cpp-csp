//
// Created by kevin on 18/06/16.
//

#ifndef CPP_CSP_MERGE_H
#define CPP_CSP_MERGE_H

#include <vector>
#include <array>
#include "../process.h"
#include "../chan.h"
#include "../par.h"

namespace csp
{
    namespace plugnplay
    {
        /*! \class merge
         * \brief Merges a stream of incoming streams into a single output stream.
         *
         * \tparam T The type that the merge process operates on.
         *
         * \author Kevin Chalmers
         * \date 26/06/2016
         */
        template<typename T>
        class merge : public process
        {
        private:

            std::vector<chan_in<T>> _ins; //<! The input channels into the process.

            chan_out<T> _out; //<! The output channel from the process.

        public:
            /*!
             * \brief Creates a new merge process.
             *
             * \param[in] ins The input channels into the process.
             * \param[in] out The output channel from the process.
             */
            merge(const std::vector<chan_in<T>> &ins, chan_out<T> out) noexcept
            : _ins(ins), _out(out)
            {
                assert(_ins.size() >= 2);
            }

            /*!
             * \brief Executes the merge process.
             */
            void run() noexcept override final
            {
                switch (_ins.size())
                {
                    // Only two input streams.
                    case 2:
                    {
                        // Execute the merge
                        T v1 = _ins[0]();
                        T v2 = _ins[1]();
                        while (true)
                        {
                            if (v1 < v2)
                            {
                                _out(v1);
                                v1 = _ins[0]();
                            }
                            else if (v1 > v2)
                            {
                                _out(v2);
                                v2 = _ins[0]();
                            }
                            else
                            {
                                _out(v1);
                                v1 = _ins[0]();
                                v2 = _ins[1]();
                            }
                        }
                    }
                    // Three input streams.  Merge two, then merge the result with the third.
                    case 3:
                    {
                        one2one_chan<T> c;
                        par
                        {
                            merge<T>({ _ins[0], _ins[1] }, c),
                            merge<T>({ c, _ins[2] }, _out)
                        }();
                        break;
                    }
                    // All other cases split the incoming streams into two sets.
                    default:
                    {
                        auto n = _ins.size() / 2;
                        // Grab bottom and top halves of channels.
                        std::vector<chan_in<T>> bottom(_ins.begin(), _ins.begin() + n);
                        std::vector<chan_in<T>> top(_ins.begin() + n, _ins.end());
                        std::array<one2one_chan<T>, 2> d;
                        // Merge two sets into a final merge process
                        par
                        {
                           merge<T>(bottom, d[0]),
                           merge<T>(top, d[1]),
                           merge<T>({ d[0], d[1] }, _out)
                        }();
                    }
                }
            }
        };
    }
}

#endif //CPP_CSP_MERGE_H
