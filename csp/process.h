//
// Created by kevin on 28/04/16.
//

#ifndef CPP_CSP_PROCESS_H
#define CPP_CSP_PROCESS_H

#include <functional>

namespace csp
{
    /*! \class process
     * \brief Interface class for declaring a class as a process.
     *
     * \author Kevin Chalmers
     *
     * \date 28/04/2016
     */
    class process
    {
    public:
        /*!
         * \brief Destroys the process object.
         */
        virtual ~process() { }

        /*!
         * \brief Called when process is run.
         */
        virtual void run() noexcept = 0;

        /*!
         * \brief Operator overload.  Calls the run method.
         */
        void operator()() noexcept { this->run(); }
    };

    /*!
     * \brief Creates a process function.
     *
     * \tparam Fun The type of the function.
     * \tparam Args The type of the argument pack.
     *
     * \param[in] f The function to use.
     * \param[in] params The parameters for the function.
     *
     * \return A function object comprising of the function and the bound parameters.
     */
    template<typename Fun, typename... Args>
    std::function<void()> make_proc(Fun &&f, Args&&... params)
    {
        return std::bind(std::forward<Fun>(f), std::forward<Args>(params)...);
    };

}

#endif //CPP_CSP_PROCESS_H
