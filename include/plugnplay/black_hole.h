//
// Created by kevin on 03/05/16.
//

#ifndef CPP_CSP_BLACK_HOLE_H
#define CPP_CSP_BLACK_HOLE_H

#include "../process.h"
#include "../chan.h"

namespace csp
{
    namespace plugnplay
    {
        /*! \class black_hole
         * \brief Process that disposes of its input
         *
         * \tparam T The type that the process operates on.
         *
         * \author Kevin Chalmers
         *
         * \date 03/05/2016
         */
        template<typename T>
        class black_hole : public process
        {
        private:

            chan_in<T> _in; //<! The input channel into the process.

        public:
            /*!
             * \brief Creates a new black hole process.
             *
             * \param[in] in The input channel into the process.
             */
            black_hole(chan_in<T> in) noexcept
            : _in(in)
            {
            }

            /*!
             * \brief Runs the black hole process.
             */
            void run() noexcept override final
            {
                while (true)
                    _in();
            }
        };
    }
}

#endif //CPP_CSP_BLACK_HOLE_H
