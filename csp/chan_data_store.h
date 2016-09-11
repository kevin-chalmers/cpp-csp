//
// Created by kevin on 26/04/16.
//

#ifndef CPP_CSP_CHAN_DATA_STORE_H
#define CPP_CSP_CHAN_DATA_STORE_H

#include <exception>
#include <memory>
#include <deque>

namespace csp
{
    /*! \enum DATA_STORE_STATE
     * \brief Used to represent the state of a channel data store.
     *
     * \author Kevin Chalmers
     *
     * \date 17/04/2016
     */
    enum class DATA_STORE_STATE
    {
        EMPTY = 0,
        NONEMPTYFULL = 1,
        FULL = 2
    };

    /*! \class chan_data_store
     * \brief Interface for a channel data store.  Used within buffered channels.
     *
     * \tparam T The type that the data store works on.
     *
     * \author Kevin Chalmers
     *
     * \date 26/04/2016
     */
    template<typename T>
    class chan_data_store
    {
    protected:
        /*! \class chan_data_store_internal
         * \brief Internal representation of a channel data store.
         *
         * \author Kevin Chalmers
         *
         * \date 26/04/2016
         */
        class chan_data_store_internal
        {
        public:
            /*!
             * \brief Adds a value to the channel data store.
             *
             * \param[in] value The value to add to the data store.
             */
            virtual void put(T value) noexcept(false) = 0;

            /*!
             * \brief Gets a value from the channel data store.
             *
             * \return The value retrieved from the channel data store.
             */
            virtual T get() noexcept(false) = 0;

            /*!
             * \brief Clears all values from the channel.
             */
            virtual void clear() noexcept = 0;

            /*!
             * \brief Gets the state of the buffer.
             *
             * \return The current state of the data store.
             */
            virtual DATA_STORE_STATE get_state() const noexcept = 0;

            /*!
             * \brief Destroys the internal channel data store.
             */
            virtual ~chan_data_store_internal() { }
        };

        std::shared_ptr<chan_data_store_internal> _internal = nullptr; //<! Pointer to the actual channel data store.

        /*!
         * \brief Protected constructor.  Used to create a chan_data_store from a pointer to an internal object.
         *
         * param[in] internal Pointer to the internal chan_data_store representation.
         */
        chan_data_store(std::shared_ptr<chan_data_store_internal> internal) noexcept
        : _internal(internal)
        {
        }

    public:
        /*!
         * \brief Copy constructor.
         *
         * \param[in] other The chan_data_store to copy.
         */
        chan_data_store(const chan_data_store<T> &other) noexcept = default;

        /*!
         * \brief Move constructor.
         *
         * \param[in] rhs The chan_data_store to copy.
         */
        chan_data_store(chan_data_store<T> &&rhs) noexcept = default;

        /*!
         * \brief Copy assignment operator.
         *
         * \param[in] other The chan_data_store to copy.
         *
         * \return A copy of the chan_data_store.
         */
        chan_data_store<T>& operator=(const chan_data_store<T> &other) noexcept = default;

        /*!
         * \brief Move assignment operator.
         *
         * \param[in] rhs The chan_data_store to copy.
         *
         * \return A copy of the chan_data_store.
         */
        chan_data_store<T>& operator=(chan_data_store<T> &&rhs) noexcept = default;

        /*!
         * \brief Destroys the chan_data_store.
         */
        virtual ~chan_data_store() noexcept { }

        /*!
         * \brief Adds a value to the channel data store.
         *
         * \param[in] value The value to add to the channel data store.
         */
        void put(T value) const noexcept { _internal->put(value); }

        /*!
         * \brief Gets a value from the channel data store.
         *
         * \return Value retrieved from the channel data store.
         */
        T get() const noexcept { return _internal->get(); }

        /*!
         * \brief Clears all values from the channel data store.
         */
        void clear() const noexcept { _internal->clear(); }

        /*!
         * \brief Gets the current state of the buffer.
         *
         * \return The current state of the buffer.
         */
        DATA_STORE_STATE get_state() const noexcept { return _internal->get_state(); }
    };

    /*! \class buffer
     * \brief A standard buffer of a fixed size.
     *
     * \tparam T The type that the buffer operates.
     *
     * \author Kevin Chalmers
     *
     * \date 26/04/2016
     */
    template<typename T>
    class buffer : public chan_data_store<T>
    {
    protected:
        /*! \class buffer_internal
         * \brief Internal implementation of a standard buffer.
         *
         * \author Kevin Chalmers
         *
         * \date 26/04/2016
         */
        class buffer_internal : public chan_data_store<T>::chan_data_store_internal
        {
        private:

            unsigned int _size = 0; //<! The size of the buffer.

            std::deque<T> _buffer; //<! The actual buffer used to store data.

        public:
            /*!
             * \brief Creates a new internal buffer.
             *
             * \param[in] size The size of the buffer to create.
             */
            buffer_internal(unsigned int size) noexcept
            : _size(size)
            {
            }

            /*!
             * \brief Adds a value to the buffer.
             *
             * \param[in] value The value to add to the buffer.
             */
            void put(T value) noexcept override final
            {
                _buffer.push_back(value);
            }

            /*!
             * \brief Gets a value from the buffer.
             *
             * \return The value removed from the buffer.
             */
            T get() noexcept override final
            {
                T to_return = _buffer.front();
                _buffer.pop_front();
                return to_return;
            }

            /*!
             * \brief Clears all values from the buffer.
             */
            void clear() noexcept override final
            {
                _buffer.clear();
            }

            /*!
             * \brief Gets the current state of the buffer.
             *
             * \return The current state of the module.
             */
            DATA_STORE_STATE get_state() const noexcept override final
            {
                if (_buffer.size() == 0) return DATA_STORE_STATE::EMPTY;
                else if (_buffer.size() == _size) return DATA_STORE_STATE::FULL;
                else return DATA_STORE_STATE::NONEMPTYFULL;
            }
        };

    public:
        /*!
         * \brief Creates a new buffer.
         *
         * \param[in] size The size of the buffer to create.
         */
        buffer(unsigned int size) noexcept
        : chan_data_store<T>(std::shared_ptr<buffer_internal>(new buffer_internal(size)))
        {
        }
    };

    /*! \class infinite_buffer
     * \brief A buffer with infinite size.
     *
     * \tparam T The type that the data store operates on.
     *
     * \author Kevin Chalmers
     *
     * \date 26/04/2016
     */
    template<typename T>
    class infinite_buffer : public chan_data_store<T>
    {
    protected:
        /*! \class infinite_buffer_internal
         * \brief Internal representation of an infinite buffer.
         *
         * \author Kevin Chalmers
         *
         * \date 26/04/2016
         */
        class infinite_buffer_internal : public chan_data_store<T>::chan_data_store_internal
        {
        private:

            std::deque<T> _buffer; //!< The actual buffer used to store data.

        public:
            /*!
             * \brief Creates a new buffer.  Used internally by the framework.
             */
            infinite_buffer_internal() noexcept { }

            /*!
             * \brief Adds a value to the buffer.
             *
             * \param[in] value The value to add to the buffer.
             */
            void put(T value) noexcept override final
            {
                _buffer.push_back(value);
            }

            /*!
             * \brief Gets a value from the buffer.
             *
             * \return Value removed from the buffer.
             */
            T get() noexcept override final
            {
                T to_return = _buffer.front();
                _buffer.pop_front();
                return to_return;
            }

            /*!
             * \brief Clears all values from the buffer.
             */
            void clear() noexcept override final
            {
                _buffer.clear();
            }

            /*!
             * \brief Gets the current state of the buffer.
             *
             * \return The current state of the buffer.
             */
            DATA_STORE_STATE get_state() const noexcept override final
            {
                if (_buffer.size() == 0) return DATA_STORE_STATE::EMPTY;
                else return DATA_STORE_STATE::NONEMPTYFULL;
            }
        };

    public:
        /*!
         * \brief Creates a new infinite buffer.
         */
        infinite_buffer() noexcept
        : chan_data_store<T>(std::shared_ptr<infinite_buffer_internal>(new infinite_buffer_internal()))
        {
        }
    };

    /*! \class overflowing_buffer
     * \brief A buffer that discards any value when it is full.
     *
     * \tparam T The type that the data store operates on.
     *
     * \author Kevin Chalmers
     *
     * \date 26/04/2016
     */
    template<typename T>
    class overflowing_buffer : public chan_data_store<T>
    {
    protected:
        /*! \class overflowing_buffer_internal
         * \brief Creates a new overflowing buffer.
         *
         * \author Kevin Chalmers
         *
         * \date 26/04/2016
         */
        class overflowing_buffer_internal : public chan_data_store<T>::chan_data_store_internal
        {
        private:

            unsigned int _size = 0; //<! The size of the buffer.

            std::deque<T> _buffer; //<! The actual buffer used to store data.

        public:
            /*!
             * \brief Creates a new internal overflowing buffer.  Used internally by the framework.
             *
             * \param[in] size The size of the overflowing buffer.
             */
            overflowing_buffer_internal(unsigned int size) noexcept
            : _size(size)
            {
            }

            /*!
             * \brief Adds a value to the buffer.
             *
             * \param[in] value The value to add to the buffer.
             */
            void put(T value) noexcept override final
            {
                // Only add values if buffer is not full.
                if (_buffer.size() < _size)
                    _buffer.push_back(value);
            }

            /*!
             * \brief Gets a value from the buffer.
             *
             * \return The value removed from the channel.
             */
            T get() noexcept override final
            {
                T to_return = _buffer.front();
                _buffer.pop_front();
                return to_return;
            }

            /*!
             * \brief Clears all values from the buffer.
             */
            void clear() noexcept override final
            {
                _buffer.clear();
            }

            /*!
             * \brief Gets the current state of the buffer.
             *
             * \return The current state of the buffer.
             */
            DATA_STORE_STATE get_state() const noexcept override final
            {
                if (_buffer.size() == 0) return DATA_STORE_STATE::EMPTY;
                else return DATA_STORE_STATE::NONEMPTYFULL;
            }
        };

    public:
        /*!
         * \brief Creates a new overflowing_buffer.
         */
        overflowing_buffer()
        : chan_data_store<T>(std::shared_ptr<overflowing_buffer_internal>(new overflowing_buffer_internal()))
        {
        }
    };

    /*! \class overwrite_oldest_buffer
     * \brief A buffer that overwrites its oldest value when full.
     *
     * \tparam T The type that the data store operates on.
     *
     * \author Kevin Chalmers
     *
     * \date 26/04/2016
     */
    template<typename T>
    class overwrite_oldest_buffer : public chan_data_store<T>
    {
    protected:
        /*! \class overwrite_oldest_buffer_internal
         * \brief Internal implementation of a overwrite oldest buffer.
         *
         * \author Kevin Chalmers
         *
         * \date 26/04/2016
         */
        class overwrite_oldest_buffer_internal : public chan_data_store<T>::chan_data_store_internal
        {
        private:

            unsigned int _size = 0; //<! The size of the buffer.

            std::deque<T> _buffer; //<! The actual buffer used to store data.

        public:
            /*!
             * \brief Creates a new internal overwrite oldest buffer.
             *
             * \param[in] size The size of the buffer.
             */
            overwrite_oldest_buffer_internal(unsigned int size) noexcept
            : _size(size)
            {
            }

            /*!
             * \brief Adds a value to the buffer.
             *
             * \param[in] value The value to put into the buffer.
             */
            void put(T value) noexcept override final
            {
                // If buffer is full, remove oldest (front) value
                if (_buffer.size() == _size)
                    _buffer.pop_front();
                _buffer.push_back(value);
            }

            /*!
             * \brief Gets a value from the buffer.
             *
             * \return The value removed from the buffer.
             */
            T get() noexcept override final
            {
                T to_return = _buffer.front();
                _buffer.pop_front();
                return to_return;
            }

            /*!
             * \brief Clears all values from the buffer.
             */
            void clear() noexcept override final
            {
                _buffer.clear();
            }

            /*!
             * \brief Gets the current state of the buffer.
             *
             * \return The current state of the buffer.
             */
            DATA_STORE_STATE get_state() const noexcept override final
            {
                if (_buffer.size() == 0) return DATA_STORE_STATE::EMPTY;
                else return DATA_STORE_STATE::NONEMPTYFULL;
            }
        };

    public:
        /*!
         * \brief Creates a new overwrite oldest buffer.
         *
         * \param[in] size The size of the buffer.
         */
        overwrite_oldest_buffer(unsigned int size) noexcept
        : chan_data_store<T>(std::shared_ptr<overwrite_oldest_buffer_internal>(size))
        {
        }
    };

    /*! \class overwriting_buffer
     * \brief A buffer that overwrites the last entry if it is full.
     *
     * \tparam T The type that the buffer oe
     *
     * \author Kevin Chalmers
     *
     * \date 28/04/2016
     */
    template<typename T>
    class overwriting_buffer : public chan_data_store<T>
    {
    protected:
        /*! \class overwriting_buffer_internal
         * \brief The internal representation of an overwriting buffer.
         *
         * \author Kevin Chalmers
         *
         * \date 28/04/2016
         */
        class overwriting_buffer_internal : public chan_data_store<T>::chan_data_store_internal
        {
        private:

            unsigned int _size = 0; //<! The size of the buffer.

            std::deque<T> _buffer; //<! The actual buffer used to store data.

        public:
            /*!
             * \brief Creates a new internal overwriting buffer.
             *
             * \param[in] size The size of the buffer.
             */
            overwriting_buffer_internal(unsigned int size) noexcept
            : _size(size)
            {
            }

            /*!
             * \brief Adds a value to the buffer.
             *
             * \param[in] value The value to put in the buffer.
             */
            void put(T value) noexcept override final
            {
                // If buffer is full, remove last item
                if (_buffer.size() == _size)
                    _buffer.pop_back();
                _buffer.push_back(value);
            }

            /*!
             * \brief Gets a value from the buffer.
             *
             * \return The value removed from the buffer.
             */
            T get() noexcept override final
            {
                T to_return = _buffer.front();
                _buffer.pop_back();
                return to_return;
            }

            /*!
             * \brief Clears all values from the buffer.
             */
            void clear() noexcept override final
            {
                _buffer.clear();
            }

            /*!
             * \brief Gets the current state of the buffer.
             *
             * \return The current state of the buffer.
             */
            DATA_STORE_STATE get_state() const noexcept final override
            {
                if (_buffer.size() == 0) return DATA_STORE_STATE::EMPTY;
                else return DATA_STORE_STATE::NONEMPTYFULL;
            }
        };

    public:
        /*!
         * \brief Creates a new overwriting buffer.
         *
         * \param[in] size The size of the buffer.
         */
        overwriting_buffer(unsigned int size) noexcept
        : chan_data_store<T>(std::shared_ptr<overwriting_buffer_internal>(size))
        {
        }
    };
}

#endif //CPP_CSP_CHAN_DATA_STORE_H
