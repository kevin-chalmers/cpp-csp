//
// Created by kevin on 12/08/16.
//

#ifndef CPP_CSP_SKELETONS_H
#define CPP_CSP_SKELETONS_H

#include <functional>

namespace csp
{
    namespace skeletons
    {
        // Forward declarations
        template<typename T>
        class connector_in;
        template<typename T>
        class connector_out;

        /*! \class connector
         * \brief A connector object
         *
         * \tparam T The type communicated by the connector.
         *
         * \author Kevin Chalmers
         *
         * \date 1/11/2016
         */
        template<typename T>
        class connector
        {
        protected:
            /*! \class connector_internal
             * \brief Internal representation of the connector.
             *
             * \author Kevin Chalmers
             *
             * \date 1/11/2016
             */
            class connector_internal
            {
            public:
                /*!
                 * \brief Sends a value via the connector.
                 *
                 * \param[in] value Value to send via the connector.
                 */
                virtual void send(T value) noexcept = 0;

                /*!
                 * \brief Receive a value from the connector.
                 *
                 * \return Value received from the connector.
                 */
                T void receive() noexcept = 0;

                /*!
                 * \brief Checks if the connector has finished sending data.
                 *
                 * \return True if the connector has completed, false otherwise.
                 */
                bool finished() const noexcept = 0;

                /*!
                 * \brief Sent by sending block to indicate it has finished processing and no further data will come.
                 */
                void finish() noexcept = 0;
            };

            std::shared_ptr<connector_internal> _internal = nullptr; //<! Pointer to internal representation of the connector.

            /*!
             * \brief Creates a new connector from an existing internal representation.
             *
             * \param[in] internal The existing internal representation of a connector.
             */
            connector(std::shared_ptr<connector_internal> internal) noexcept
            : _internal(internal)
            {
            }

        public:

            /*!
             * \brief Sends a value via the connector.
             *
             * \param[in] value Value to send via the connector.
             */
            void send(T value) const noexcept { _internal->send(value); }

            /*!
             * \brief Receive a value from the connector.
             *
             * \return Value received from the connector.
             */
            T receive() const noexcept { return _internal->receive(); }

            /*!
             * \brief Checks if the connector has finished sending data.
             *
             * \return True if the connector has completed, false otherwise.
             */
            bool finished() const noexcept { return _internal->finished(); };

            /*!
             * \brief Sent by sending block to indicate it has finished processing and no further data will come.
             */
            void finish() const noexcept { _internal->finish(); };

            /*!
             * \brief Gets the receiving end (connector in) of the connector.
             *
             * \return An object allowing receive actions from the connector.
             */
            connector_in<T> get_input() const noexcept;

            /*!
             * \brief Gets the sending end (connector out) of the connector.
             *
             * \return An object allowing send actions to the connector.
             */
            connector_out<T> get_output() const noexcept;

            /*!
             * \brief Implicit conversion operator, converting the connector into its input end.
             *
             * \return An object allowing receive actions from the connector.
             */
            operator connector_in<T>() const noexcept { return get_input(); }

            /*!
             * \brief Implicit conversion operator, converting the connector into its output end.
             *
             * \return An object allowing send actions to the connector.
             */
            operator connector_out<T>() const noexcept { return get_output(); }
        };

        /*! \class connector_in
         * \brief The input end (for reading/recieving messages) of a connector.
         *
         * \tparam T The type that can be received from the connector.
         *
         * \author Kevin Chalmers
         *
         * \date 1/11/2016
         */
        template<typename T>
        class connector_in
        {
        private:
            connector<T> _conn; //<! The connector being used for actual communication.
        public:
            /*!
             * \brief Creates a new connector_in from an existing connector.
             */
            connector_in(connector<T> conn) noexcept
            : _conn(conn)
            {
            }

            /*!
             * \brief Receives a value from the connector.
             *
             * \return The value received from the connector.
             */
            T receive() const noexcept { return std::move(_conn.receive()); }

            /*!
             * \brief Receives a value from the connector.
             *
             * \return The value received from the connector.
             */
            T operator()() const noexcept { return std::move(_conn.receive()); }

            /*!
             * \brief Checks if the connector has finished sending values.
             *
             * \return true if the connector has completed, false otherwise.
             */
            bool finished() const { return _conn->finished(); }
        };

        /*! \class connector_out
         * \brief The output end (for writing/sending messages) of a connector.
         *
         * \tparam T The type that can be sent by the connector.
         *
         * \author Kevin Chalmers
         *
         * \date 1/11/2016
         */
        template<typename T>
        class connector_out
        {
        private:
            connector<T> _conn; //<! The connector being used for actual communication.
        public:
            /*!
             * \brief Creates a new connector_out from an existing connector.
             */
            connector_out(connector<T> conn) noexcept
            : _conn(conn)
            {
            }

            /*!
             * \brief Sends a value to the connector.
             *
             * \param[in] value The value to send via the connector.
             */
            void send(T value) const noexcept { _conn.send(std::move(value)); }

            /*!
             * \brief Sends a value to the connector.
             *
             * \param[in] value The value to send via the connector.
             */
            T operator()(T value) const noexcept { _conn.send(std::move(value)); }

            /*!
             * \brief Informs the connector that no further data will be sent.
             */
            bool finished() const noexcept { _conn.finish(); }
        };

        /* \class block
         * \brief Base building block for a skeleton application.
         *
         * \tparam IN The input type into the block.
         * \tparam OUT The output type from the block
         *
         * \author Kevin Chalmers
         *
         * \date 12/08/2016
         */
        template<typename IN, typename OUT>
        class block : public process
        {
        protected:
            /* \class block_internal
             * \brief Internal representation of a block.
             *
             * \author Kevin Chalmers
             *
             * \date 12/08/2016
             */
            class block_internal
            {
            private:
                connector_in<IN> recv; //<! The incoming connector to the block.

                connector_out<OUT> send; //<! The outgoing connector form the block.

            public:

                virtual void run() noexcept = 0;
            };

            std::shared_ptr<block_internal> _internal = nullptr; //<! Pointer to internal representation.

        public:

        };

        template<typename IN, typename OUT, unsigned int N>
        class group : public block<std::array<IN, N>, std::array<OUT, N>>
        {

        };

        template<typename OUT>
        class source : public block<void, OUT>
        {
        protected:
            class source_internal : public block<void, OUT>::block_internal
            {


            public:

            };
        };

        template<typename IN>
        class sink : public block<IN, void>
        {

        };

        template<typename IN, typename OUT>
        class wrapper : public block<IN, OUT>
        {
        protected:
            class wrapper_internal : public block<IN, OUT>::block_internal
            {
            private:
                connector_in<IN> in;
                connector_out<OUT> out;
                std::function<OUT(IN)> fun;

                void run() noexcept
                {
                    while (!in.finished())
                    {
                        auto value = in();
                        out(fun(value));
                    }
                }
            };
        };

        template<typename IN, typename OUT, unsigned int N>
        class broadcast : public block<IN, std::array<OUT, N>>
        {

        };

        template<typename IN, typename OUT>
        class functional : public block<IN, OUT>
        {

        };

        template<typename IN, typename OUT>
        class par_block : public functional<IN, OUT>
        {

        };

        template<typename IN, typename OUT>
        class par_n_block : public functional<IN, OUT>
        {

        };

        template<typename IN, typename OUT>
        class pipeline : public functional<IN, OUT>
        {

        };

        template<typename IN, typename OUT>
        class spread : public functional<IN, OUT>
        {

        };

        template<typename IN, typename OUT>
        class reduce : public functional<IN, OUT>
        {

        };

        template<typename IN, typename OUT>
        class one2one_connector : public connector<IN, OUT>
        {

        };

        template<typename T>
        class feedback : public one2one_connector<T>, public block<T, T>
        {

        };

        template<typename T>
        class one2any_connector : public connector<T>
        {

        };

        template<typename T>
        class unicast_auto : public one2any_connector<T>, public block<T, T>
        {

        };

        template<typename IN, typename OUT>
        class any2one_connector : public connector<IN, OUT>
        {

        };

        template<typename IN, typename OUT>
        class any2any_connector : public connector<IN, OUT>
        {

        };

        template<typename IN, typename OUT>
        class many2one_connector : public connector<IN, OUT>
        {

        };

        template<typename IN, typename OUT>
        class gather : public many2one_connector<IN, OUT>, public block<IN, OUT>
        {

        };

        template<typename IN, typename OUT>
        class gatherall : public many2one_connector<IN, OUT>, public block<IN, OUT>
        {

        };

        template<typename IN, typename OUT>
        class one2many_connector : public connector<IN, OUT>
        {

        };

        template<typename IN, typename OUT>
        class broadcast : public one2many_connector<IN, OUT>, public block<IN, OUT>
        {

        };

        template<typename IN, typename OUT>
        class scatter : public one2many_connector<IN, OUT>, public block<IN, OUT>
        {

        };

        template<typename IN, typename OUT>
        class unicast_rr : public one2many_connector<IN, OUT>, public block<IN, OUT>
        {

        };
    }
}

#endif //CPP_CSP_SKELETONS_H
