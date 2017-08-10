//
// Created by kevin on 02/05/16.
//

#ifndef CPP_CSP_PATTERNS_H_H
#define CPP_CSP_PATTERNS_H_H

#include <vector>
#include <initializer_list>
#include <cassert>
#include "chan.h"
#include "par.h"

namespace csp
{
    /*!
     * \brief Executes a parallel across a collection of data.
     *
     * \tparam RanIt Type of the data range.
     * \tparam Fun Type of the function.
     *
     * \param[in] begin Beginning of the range of data.
     * \param[in] end End of the range of data.
     * \param[in] f The function to execute on the data.
     */
    template<typename RanIt, typename Fun>
    void par_for(RanIt begin, RanIt end, Fun &&f) noexcept
    {
        // Create vector of functions.
        std::vector<std::function<void()>> procs;
        for (RanIt data = begin; data != end; ++data)
            procs.push_back(std::bind(f, *data));
        // Create a par with the functions
        par p(procs);
        // Run the par
        p();
    }

    /*
     * \brief Executes a n copies of the function in parallel.
     *
     * \param[in] n The number of copies of the function to run.
     * \param[in] f The function to execute.
     */
    void par_for_n(size_t n, std::function<void()> &&f) noexcept
    {
        // Create vector of functions.
        std::vector<std::function<void()>> procs;
        for (size_t i = 0; i < n; ++i)
            procs.push_back(f);
        // Create a par with the functions
        par p(procs);
        // Run the par
        p();
    }

    /*!
     * \brief Performs a parallel read across an initializer list of channels.
     *
     * \tparam T The type the channels operate on.
     *
     * \param[in] chans The channels to parallel read across.
     *
     * \return A vector of read in values.
     */
    template<typename T>
    std::vector<T> par_read(std::initializer_list<chan_in<T>> &&chans) noexcept
    {
        // Create vector of channels.
        std::vector<chan_in<T>> channels(chans);
        // Vector to read values into
        std::vector<T> values(channels.size());
        // Create vector of processes to run
        std::vector<std::function<void()>> procs(channels.size());
        unsigned int i = 0;
        for (auto &c : channels)
        {
            procs[i] = [=, &c, &values](){ values[i] = c(); };
            ++i;
        }
        // Run parallel
        par p(procs);
        p();
        // Return values
        return values;
    }

    /*!
     * \brief Performs a parallel read across a vector of channels.
     *
     * \tparam T The type the channels operate on.
     *
     * \param[in] chans The channels to read from.
     *
     * \return A vector of read in values.
     */
    template<typename T>
    std::vector<T> par_read(std::vector<chan_in<T>> &chans) noexcept
    {
        // Vector to read values into
        std::vector<T> values(chans.size());
        // Create vector of processes to run
        std::vector<std::function<void()>> procs;
        unsigned int i = 0;
        for (auto &c : chans)
        {
            procs[i] = [=, &c, &values]() { values[i] = c(); };
            ++i;
        }
        // Run parallel
        par p(procs);
        p();
        // Return values
        return values;
    }

    /*!
     * \brief Performs a parallel read across a range of channels.
     *
     * \tparam T The type the channels operate on.
     * \tparam RanIt The type of the channel range.
     *
     * \param[in] begin The beginning of the channel range.
     * \param[in] end The end of the channel range.
     */
    template<typename T, typename RanIt>
    std::vector<T> par_read(RanIt begin, RanIt end) noexcept
    {
        static_assert(std::iterator_traits<RanIt>::value_type == typeid(chan_in<T>), "par_read requires a collection of chan_in");
        // Vector of channels
        std::vector<chan_in<T>> chans(begin, end);
        // Vector to read values into
        std::vector<T> values(chans.size());
        // Create vector of processes to run
        std::vector<std::function<void()>> procs(chans.size());
        unsigned int i = 0;
        for (auto &c : chans)
        {
            procs[i] = [=, &c, &values](){ values[i] = c(); };
            ++i;
        }
        // Run parallel
        par p(procs);
        p();
        // Return values
        return values;
    }

    /*!
     * \brief Performs a parallel write across an initializer list of channels.
     *
     * \tparam T The type the channels operate on.
     *
     * \param[in] chans The channels to write to.
     * \param[in] values The values to write to the channels.
     */
    template<typename T>
    void par_write(std::initializer_list<chan_out<T>> &&chans, const std::vector<T> &values) noexcept
    {
        assert(chans.size() == values.size());
        // Create vector of channels
        std::vector<chan_out<T>> channels(chans);
        // Create vector of processes to run
        std::vector<std::function<void()>> procs(channels.size());
        unsigned int i = 0;
        for (auto &c : channels)
        {
            procs[i] = [=, &c, &values]() { c(values[i]); };
            ++i;
        }
        // Run parallel
        par p(procs);
        p();
    }

    /*!
     * \brief Performs a prallel write across a vector list of channels.
     *
     * \tparam T The type that the channels operate on.
     *
     * \param[in] chans Channels to write to.
     * \param[in] values Values to write to the channels.
     */
    template<typename T>
    void par_write(std::vector<chan_out<T>> &chans, const std::vector<T> &values) noexcept
    {
        assert(chans.size() == values.size());
        // Create vector of processes to run
        std::vector<std::function<void()>> procs(chans.size());
        unsigned int i = 0;
        for (auto &c : chans)
        {
            procs[i] = [=, &c, &values](){ c(values[i]); };
            ++i;
        }
        // Run parallel
        par p(procs);
        p();
    }

    /*!
     * \brief Performs a parallel write across a range of channels.
     *
     * \tparam T The type that the channels operate on.
     * \tparam RanIt The type of the channel range.
     *
     * \param[in] begin The start of the channel range.
     * \param[in] end The end of the channel range.
     * \param[in] values The values to write to the channels.
     */
    template<typename T, typename RanIt>
    void par_write(RanIt begin, RanIt end, const std::vector<T> &values) noexcept
    {
        static_assert(std::iterator_traits<RanIt>::value_type == typeid(chan_out<T>), "par_write requires a collection of chan_out");
        std::vector<chan_out<T>> chans(begin, end);
        assert(chans.size() == values.size());
        // Create a vector of processes to run
        std::vector<std::function<void()>> procs(chans.size());
        unsigned int i = 0;
        for (auto &c : chans)
        {
            procs[i] = [=, &c, &values](){ c(values[i]); };
            ++i;
        }
        // Run parallel
        par p(procs);
        p();
    }
}



#endif //CPP_CSP_PATTERNS_H_H
