#pragma once

#include <memory>
#include <vector>
#include <mutex>
#include "../util.hpp"
#include "alternative.hpp"

namespace csp
{
	template<typename T, bool POISONABLE = false>
	class channel_internal : public guard_internal
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

		virtual bool enable_reader(alt_internal *alt) noexcept = 0;

		virtual bool disable_reader() noexcept = 0;

		virtual bool enable_writer(alt_internal *alt) noexcept = 0;

		virtual bool disable_writer() noexcept = 0;

        bool enable(alt_internal *alt) noexcept { return this->enable_reader(alt); }

        virtual bool disable() noexcept { return this->disable_reader(); }

		virtual bool reader_pending() noexcept = 0;

		virtual bool writer_pending() noexcept = 0;

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

        inline bool enable_reader(alt_internal *alt) const noexcept
        {
            return _internal->enable_reader(alt);
        }

        inline bool disable_reader() const noexcept
        {
            return _internal->disable_reader();
        }

        inline bool enable_writer(alt_internal const* alt) const noexcept
        {
            return _internal->enable_writer(alt);
        }

        inline bool disable_writer() const noexcept
        {
            return _internal->disable_writer();
        }

        inline bool reader_pending() const noexcept
        {
            return _internal->reader_pending();
        }

        inline bool writer_pending() const noexcept
        {
            return _internal->writer_pending();
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
            write(move(value));
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
	class channel_input_internal final : public guard_internal
	{
	protected:
		std::unique_ptr<channel_end_mutex> _mut = nullptr;

		channel<T, POISONABLE> _chan = nullptr;
	public:
		channel_input_internal(channel<T, POISONABLE> chan, std::unique_ptr<channel_end_mutex> mut = std::make_unique<unshared_channel_end_mutex>())
		: _mut(move(mut)), _chan(chan)
		{
		}

		~channel_input_internal() = default;

		inline T read() const
		{
			std::lock_guard<channel_end_mutex> lock(*_mut);
			return _chan.read();
		}

		inline T start_read() const
		{
			_mut->lock();
			return _chan.start_read();
		}

		inline void end_read() const
		{
			_mut->unlock();
		}

		inline bool enable(alt_internal *alt) noexcept
		{
			return _chan.enable_reader(alt);
		}

		inline bool disable() noexcept
		{
			return _chan.disable_reader();
		}

		inline bool pending() const
		{
			return _chan.reader_pending();
		}

		inline void poison(size_t strength) noexcept
		{
			std::lock_guard<channel_end_mutex> lock(_mut);
			_chan.reader_poison(strength);
		}
	};

	template<typename T, bool POISONABLE = false>
	class chan_in
	{
	protected:
		std::shared_ptr<channel_input_internal<T, POISONABLE>> _internal = nullptr;
	public:
		explicit chan_in(channel<T, POISONABLE> chan)
		: _internal(std::make_shared<channel_input_internal<T, POISONABLE>>(chan))
		{
		}

		chan_in(channel<T, POISONABLE> chan, std::unique_ptr<channel_end_mutex> mut)
		: _internal(std::make_shared<channel_input_internal<T, POISONABLE>>(chan, move(mut)))
		{
		}

		virtual ~chan_in() = default;

		chan_in(const chan_in<T, POISONABLE>&) = default;

		chan_in(chan_in<T, POISONABLE>&&) = default;

		chan_in<T, POISONABLE>&operator=(const chan_in<T, POISONABLE>&) = default;

		chan_in<T, POISONABLE>&operator=(chan_in<T, POISONABLE>&&) = default;

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

		inline T operator()() const
		{
			return _internal->read();
		}
	};

	template<typename T, bool POISONABLE = false>
	class guarded_chan_in final : public chan_in<T, POISONABLE>, public guard
    {
    public:
        explicit guarded_chan_in(channel<T, POISONABLE> chan)
        : chan_in<T, POISONABLE>(chan)
        {
            _guard_internal = this->_internal;
        }

        guarded_chan_in(channel<T, POISONABLE> chan, std::unique_ptr<channel_end_mutex> mut)
        : chan_in<T, POISONABLE>(chan, mut)
        {
            _guard_internal = this->_internal;
        }

        guarded_chan_in() = default;

        guarded_chan_in(const guarded_chan_in<T, POISONABLE>&) = default;

        guarded_chan_in(guarded_chan_in<T, POISONABLE>&&) = default;

        guarded_chan_in<T, POISONABLE>&operator=(const guarded_chan_in<T, POISONABLE>&) = default;

        guarded_chan_in<T, POISONABLE>&operator=(guarded_chan_in<T, POISONABLE>&&) = default;

        inline bool enable(alt_internal *alt) noexcept
        {
            return this->_internal->enable(alt);
        }

        inline bool disable() noexcept
        {
            return this->_internal->disable();
        }
    };

	template<typename T, bool POISONABLE = false>
	class channel_output_internal
	{
	protected:
		std::unique_ptr<channel_end_mutex> _mut = nullptr;

		channel<T, POISONABLE> _chan = nullptr;
	public:
		channel_output_internal(channel<T, POISONABLE> chan, std::unique_ptr<channel_end_mutex> mut = std::make_unique<unshared_channel_end_mutex>())
		: _mut(move(mut)), _chan(chan)
		{
		}

		~channel_output_internal() = default;

		inline void write(T value) const
		{
			std::lock_guard<channel_end_mutex> lock(*_mut);
			_chan.write(value);
		}

		template<typename T_ = T, typename = IsNotReference<T_>>
		inline void write(T&& value) const
		{
			std::lock_guard<channel_end_mutex> lock(*_mut);
			_chan.write(std::move(value));
		}

		inline void poison(size_t strength) noexcept
		{
			std::lock_guard<channel_end_mutex> lock(_mut);
			_chan.writer_poison(strength);
		}
	};

	template<typename T, bool POISONABLE = false>
	class chan_out
	{
	private:
		std::shared_ptr<channel_output_internal<T, POISONABLE>> _internal = nullptr;
	public:
		explicit chan_out(channel<T, POISONABLE> chan)
		: _internal(std::make_shared<channel_output_internal<T, POISONABLE>>(chan))
		{
		}

		chan_out(channel<T, POISONABLE> chan, std::unique_ptr<channel_end_mutex> mut)
		: _internal(std::make_shared<channel_output_internal<T, POISONABLE>>(chan, move(mut)))
		{
		}

		virtual ~chan_out() = default;

		chan_out(const chan_out<T, POISONABLE>&) = default;

		chan_out(chan_out<T, POISONABLE>&&) = default;

		chan_out<T, POISONABLE>&operator=(const chan_out<T, POISONABLE>&) = default;

		chan_out<T, POISONABLE>&operator=(chan_out<T, POISONABLE>&&) = default;

		inline void write(T value) const
		{
			_internal->write(value);
		}

		template<typename T_ = T, typename = IsNotReference<T_>>
		inline void write(T&& value) const
		{
			_internal->write(move(value));
		}

		inline void poison(size_t strength) noexcept
		{
			_internal->poison(strength);
		}

		inline void operator()(T value) const
		{
			_internal->write(value);
		}

		template<typename T_ = T, typename = IsNotReference<T_>>
		inline void operator()(T &&value) const
		{
			_internal->write(std::move(value));
		}
	};

	template<typename T, bool POISONABLE, template<typename, bool> typename INPUT_END, template<typename, bool> typename OUTPUT_END>
	class chan_type
	{
	private:
		INPUT_END<T, POISONABLE> _input;
		OUTPUT_END<T, POISONABLE> _output;
		channel<T, POISONABLE> _chan;
	public:
		explicit chan_type(channel<T, POISONABLE> chan)
		: _chan(chan), _input(chan), _output(chan)
		{
		}

		chan_type(channel<T, POISONABLE> chan, std::unique_ptr<channel_end_mutex> &&in_mut, std::unique_ptr<channel_end_mutex> &&out_mut)
		: _chan(chan), _input(move(in_mut), chan), _output(out_mut, chan)
		{
		}

		chan_type(const chan_type<T, POISONABLE, INPUT_END, OUTPUT_END>&) = default;

		chan_type(chan_type<T, POISONABLE, INPUT_END, OUTPUT_END>&&) = default;

		~chan_type() = default;

		chan_type<T, POISONABLE, INPUT_END, OUTPUT_END>&operator=(const chan_type<T, POISONABLE, INPUT_END, OUTPUT_END>&) = default;

		chan_type<T, POISONABLE, INPUT_END, OUTPUT_END>&operator=(chan_type<T, POISONABLE, INPUT_END, OUTPUT_END>&) = default;

		inline INPUT_END<T, POISONABLE> in() const noexcept { return _input; }

		inline OUTPUT_END<T, POISONABLE> out() const noexcept { return _output; }

		inline operator INPUT_END<T, POISONABLE>() const noexcept { return _input; }

		inline operator OUTPUT_END<T, POISONABLE>() const noexcept { return _output; }

		inline T operator()() const noexcept { return _chan.read(); }

		inline void operator()(T value) const noexcept { _chan.write(value); }

		template<typename T_ = T, typename = IsNotReference<T_>>
		inline void operator()(T &&value) const noexcept { _chan.write(move(value)); }
	};

	template<typename T, bool POISONABLE = false>
	using one2one_chan = chan_type<T, POISONABLE, guarded_chan_in, chan_out>;
}