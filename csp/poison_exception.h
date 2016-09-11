//
// Created by kevin on 06/04/16.
//

#ifndef CPP_CSP_POISON_EXCEPTION_H
#define CPP_CSP_POISON_EXCEPTION_H

#include <exception>
#include <sstream>

/*! \namespace csp
 * \brief Core namespace of CSP primitives build in C++
 *
 * This namespace contains the main collection of base classes
 * used to support CSP style concurrency in C++.  The set of classes
 * provide the ability create processes, run these processes in parallel
 * and define the communication (via message passing channels) between
 * the processes.
 */
namespace csp
{
    /*! \class poison_exception
     * \brief Exception thrown when a channel is used after being
     * poisoned.
     *
     * This exception is thrown by a channel on any operation performed
     * after the channel has been poisoned.  Poison is used to enable
     * process network termination by passing a message through a collection
     * of processes.
     *
     * \author Kevin Chalmers
     * \date 7/4/2016
     */
    class poison_exception : public std::exception
    {
    private:
        unsigned int _strength; //!< The strength of the poison in the channel
    public:
        /*!
         * \brief Creates a new poison exception instance
         *
         * \param[in] strength The strength of the poison from the channel
         */
        poison_exception(unsigned int strength) noexcept
        : _strength(strength)
        {
        }

        /*!
         * \brief Gets the message related to the error
         *
         * Will return "Channel has been poisoned".
         *
         * \return A constant char pointer (a string) "Channel has been poisoned"
         */
        const char* what() const throw()
        {
            return "Channel has been poisoned";
        }

        /*!
         * \brief Gets the strength of the poison associated with the exception
         *
         * \return A value representing the strength of the poison associated with the
         * exception
         */
        unsigned int get_strength() const noexcept { return _strength; }
    };
}

#endif //CPP_CSP_POISON_EXCEPTION_H
