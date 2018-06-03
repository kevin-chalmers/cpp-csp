#pragma once

#include <memory>
#include <vector>
#include "../util.hpp"

namespace csp
{
	template<typename T, bool POISONABLE = false>
	class channel_internal
	{
	protected:
		channel_internal() = default;
	public:
        channel_internal(const channel_internal<T, POISONABLE>&) = default;

        channel_internal(channel_internal<T, POISONABLE>&&) = default;

        channel_internal<T>&operator=(const channel_internal<T, POISONABLE>&) = default;

        channel_internal<T>&operator=(channel_internal<T, POISONABLE>&&) = default;

        virtual ~channel_internal() = default;

		virtual void write(T) = 0;

		virtual T read() = 0;

		virtual T start_read() = 0;

		virtual void end_read() = 0;

		virtual bool enable() noexcept = 0;

		virtual bool disable() noexcept = 0;

		virtual bool pending() noexcept = 0;

        virtual void reader_poison(size_t) noexcept = 0;

        virtual void writer_poison(size_t) noexcept = 0;
	};

	template<typename T, bool POISONABLE = false>
	class channel
	{
	private:
        std::shared_ptr<channel_internal<T, POISONABLE>> _internal = nullptr;
	public:
		explicit channel(std::shared_ptr<channel_internal<T, POISONABLE>> internal)
        : _internal(std::move(internal))
		{
		}

		channel(const channel<T, POISONABLE>&) = default;

		channel(channel<T, POISONABLE>&&) = default;

		~channel() = default;

		channel<T, POISONABLE>& operator=(const channel<T, POISONABLE>&) noexcept = default;

		channel<T, POISONABLE>& operator=(channel<T, POISONABLE>&&) noexcept = default;

        inline void write(T value) const
        {
            _internal->write(value);
        }

        template<typename T_ = T, typename = IsNotReference<T_>>
        inline void write(T&& value) const
        {
            _internal->write(value);
        }

        inline T read() const
        {
            return std::move(_internal->read());
        }

        inline T start_read() const
        {
            return std::move(_internal->start_read());
        }

        inline void end_read() const
        {
            _internal->end_read();
        }

        inline bool enable() noexcept
        {
            return _internal->enable();
        }

        inline bool disable() noexcept
        {
            return _internal->disable();
        }

        inline bool pending() const noexcept
        {
            return _internal->pending();
        }

        inline void reader_poison(size_t strength) noexcept
        {
            static_assert(POISONABLE);
            _internal->reader_poison(strength);
        }

        inline void writer_poison(size_t strength) noexcept
        {
            static_assert(POISONABLE);
            _internal->writer_poison(strength);
        }

		inline void operator()(T value) const
		{
			write(value);
		}

		template<typename T_ = T, typename = IsNotReference<T>>
        inline void operator()(T&& value) const
        {
            write(value);
        }

		inline T operator()() const
		{
			return std::move(read());
		}
	};
/*
	template<typename T, 
			 typename IMPLEMENTATION = thread_implementation,
			 typename CHANNEL_TYPE = IMPLEMENTATION::channel<T>,
		     typename SHARING_POLICY = unshared>
	class channel_input : public SHARING_POLICY
	{
	private:
		CHANNEL_TYPE _chan = nullptr;
	public:
		channel_input(CHANNEL_TYPE chan)
			: _chan(chan)
		{
		}

		T read() const
		{
			SHARING_POLICY::lock();
			auto to_return = _chan.read();
			SHARING_POLICY::unlock();
			return std::move(to_return);
		}
	};
*/
}