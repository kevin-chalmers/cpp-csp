#pragma once

#include <memory>
#include <vector>
#include <mutex>
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

		template<typename T_ = T, typename = IsNotReference<T_>>
        inline void operator()(T&& value) const
        {
            write(value);
        }

		inline T operator()() const
		{
			return std::move(read());
		}
	};

	struct channel_end_mutex
	{
		virtual ~channel_end_mutex() = default;

		inline virtual void lock() = 0;

		inline virtual void unlock() = 0;
	};

	struct unshared_channel_end_mutex final : public channel_end_mutex
	{
		inline void lock() { }

		inline void unlock() { }
	};

	template<typename T, bool POISONABLE = false>
	class channel_input_internal
	{
	private:
		std::unique_ptr<channel_end_mutex> _mut = nullptr;

		std::shared_ptr<channel<T, POISONABLE>> _chan = nullptr;
	public:
		channel_input_internal(std::unique_ptr<channel_end_mutex> &&mut, std::shared_ptr<channel<T, POISONABLE>> chan)
		: _mut(mut), _chan(chan)
		{
		}

		~channel_input_internal() = default;

		inline T read() const
		{
			std::lock_guard<channel_end_mutex> lock(*_mut);
			return _chan->read();
		}

		inline T start_read() const
		{
			_mut->lock();
			return _chan->start_read();
		}

		inline void end_read() const
		{
			_mut->unlock();
		}

		inline bool enable() const
		{
			return _chan->enable();
		}

		inline bool disable() const
		{
			return _chan->disable();
		}

		inline bool pending() const
		{
			return _chan->pending();
		}

		inline void poison(size_t strength) noexcept
		{
			std::lock_guard<channel_end_mutex> lock(_mut);
			_chan->reader_poison(strength);
		}
	};

	template<typename T, bool POISONABLE = false>
	class channel_input
	{
	private:
		std::shared_ptr<channel_input_internal<T, POISONABLE>> _internal = nullptr;
	public:
		channel_input(std::unique_ptr<channel_end_mutex> &&mut, std::shared_ptr<channel<T, POISONABLE>> chan)
		: _internal(std::make_shared<channel_input_internal<T, POISONABLE>>(std::move(mut), chan))
		{
		}

		virtual ~channel_input() = default;

		channel_input(const channel_input<T, POISONABLE>&) = default;

		channel_input(channel_input<T, POISONABLE>&&) = default;

		channel_input<T, POISONABLE>&operator=(const channel_input<T, POISONABLE>&) = default;

		channel_input<T, POISONABLE>&operator=(channel_input<T, POISONABLE>&&) = default;

		inline T read() const
		{
			return _internal->read();
		}

		inline T start_read() const
		{
			return _internal->start_read();
		}

		inline void end_read() const
		{
			_internal->end_read();
		}

		inline void poison(size_t strength) noexcept
		{
			_internal->poison(strength);
		}
	};
}