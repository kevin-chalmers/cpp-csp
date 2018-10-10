#pragma once

#include <memory>
#include <atomic>
#include <chrono>
#include <cassert>
#include "channel.hpp"
#include "thread_implementation.hpp"

namespace csp
{
    namespace atomic_implementation
    {
        template<typename T, bool POISONABLE = false>
        class channel_type final : public csp::channel_internal<T, POISONABLE>
        {
        private:
            std::atomic<T> _hold;
            std::atomic<bool> _reading;
            std::atomic<bool> _writing;
            std::atomic<alt_internal*> _alt;
            std::atomic<unsigned int> _strength;
        public:
            channel_type()
            : _reading(false), _writing(false), _alt(nullptr), _strength(0)
            {
            }

            void write(T value)
            {
                // Store value
                _hold.store(value, std::memory_order_relaxed);
                // Set writing to true - will inform reading end if spinning.
                _writing.store(true, std::memory_order_release);
                // Now check if an alt has occurred at some point prior to now.
                if (_alt.load(std::memory_order_acquire) != nullptr)
                    this->schedule(_alt);
                // Now spin waiting for reader to grab the date
                while (!_reading.load(std::memory_order_acquire));
                // At this point, we can check if we have been poisoned.  Poisoning will leave reading as true.  Therefore,
                // if poisoning happened, it happened before now.
                if (_strength.load(std::memory_order_relaxed) > 0)
                    throw poison_exception(_strength.load(std::memory_order_relaxed));
                // At this point, we know that the reader has the value.  It will at some point wait for us to signal
                // we are finished (it may already be waiting for that).  Set writing to false to signal.
                _writing.store(false, std::memory_order_release);
                // Now we need to wait until the reader has seen that we are finished.
                while (_reading.load(std::memory_order_acquire));
                // We have now completed.  The state visible to each thread should be consistent enough to continue without
                // a hazard.
            }

            template<typename _T = T, IsNotReference<_T>>
            void write(T &&value)
            {
                // Store value
                _hold.store(value, std::memory_order_relaxed);
                // Set writing to true - will inform reading end if spinning.
                _writing.store(true, std::memory_order_release);
                // Now check if an alt has occurred at some point prior to now.
                if (_alt.load(std::memory_order_acquire) != nullptr)
                    this->schedule(_alt);
                // Now spin waiting for reader to grab the date
                while (!_reading.load(std::memory_order_acquire));
                // At this point, we can check if we have been poisoned.  Poisoning will leave reading as true.  Therefore,
                // if poisoning happened, it happened before now.
                if (_strength.load(std::memory_order_relaxed) > 0)
                    throw poison_exception(_strength.load(std::memory_order_relaxed));
                // At this point, we know that the reader has the value.  It will at some point wait for us to signal
                // we are finished (it may already be waiting for that).  Set writing to false to signal.
                _writing.store(false, std::memory_order_release);
                // Now we need to wait until the reader has seen that we are finished.
                while (_reading.load(std::memory_order_acquire));
                // We have now completed.  The state visible to each thread should be consistent enough to continue without
                // a hazard.
            }

            T read()
            {
                // Spin for the writer.  Cannot progress until writer has started.
                while (!_writing.load(std::memory_order_acquire));
                // At this point, we can check if we have been poisoned.  Poisoning will leave writing as true.  Therefore,
                // if poisoning happened, it happened before now.
                if (_strength.load(std::memory_order_relaxed) > 0)
                    throw poison_exception(_strength.load(std::memory_order_relaxed));
                // At this point, we know the writer has at least stored the value in the hold.  So let's grab it.
                auto to_return = _hold.load(std::memory_order_relaxed);
                // Now we need to inform the writer that we have grabbed the value.  It will at some point wait for us to
                // signal we have done so (it may already be waiting for that).  Set reading to true to signal.
                _reading.store(true, std::memory_order_release);
                // Now we need to wait for the writer to signal that it has seen we have grabbed the value, and signal
                // that it has effectively completed.
                while (_writing.load(std::memory_order_acquire));
                // The writer will at some point be waiting for us to signal that we have also completed.
                _reading.store(false, std::memory_order_release);
                // We have now completed.  The state visible to each thread should be consistent enough to continue without
                // a hazard.
                return to_return;
            }

            T start_read()
            {
                // Sanity check - make sure we are not reading.
                if (_reading.load(std::memory_order_acquire))
                    throw std::logic_error("Channel already in extended read");
                // Spin for the writer.  Cannot progress until writer has started.
                while (!_writing.load(std::memory_order_acquire));
                // At this point, we can check if we have been poisoned.  Poisoning will leave writing as true.  Therefore,
                // if poisoning happened, it happened before now.
                if (_strength.load(std::memory_order_relaxed) > 0)
                    throw poison_exception(_strength.load(std::memory_order_relaxed));
                // At this point, we know the writer has at least stored the value in the hold.  So let's grab it.
                auto to_return = _hold.load(std::memory_order_relaxed);
                // Now we need to inform the writer that we have grabbed the value.  It will at some point wait for us to
                // signal we have done so (it may already be waiting for that).  Set reading to true to signal.
                _reading.store(true, std::memory_order_release);
                // Now we need to wait for the writer to signal that it has seen we have grabbed the value, and signal
                // that it has effectively completed.
                while (_writing.load(std::memory_order_acquire));
                // In a normal read, we would now inform the writer.  However, we are extended.  Just return the value.
                return to_return;
            }

            void end_read()
            {
                // Ensure we are in an extended read.
                if (!_reading.load(std::memory_order_acquire))
                    throw std::logic_error("Channel not in extended read");
                // At this point, all we really need to do is set reading to false.  The writer is currently spinning
                // waiting for this value (or at least will be if not already).
                _reading.store(false, std::memory_order_release);
            }

            bool enable_reader(alt_internal *alt) noexcept
            {
                // Set alt
                _alt.store(alt, std::memory_order_release);
                // Check if writing
                auto local_writing = _writing.load(std::memory_order_acquire);
                // At this point we know one of the following:
                // 1. local_writing is true, and the write has committed before seeing alt
                // 2. local_writing is true, and the channel is poisoned
                // 3. local_writing is true, and the write saw alt before committing
                // 4. local_writing is false, and the write has not committed
                // Deal with 2 first
                // Check if poisoned
                if (_strength.load(std::memory_order_relaxed) > 0)
                    return true;
                // 1 & 3 mean we are in a committed write.
                // 4 will be dealt with by memory ordering.  The write will see alting as true and act accordingly.
                // In both cases, local_writing contains the value to return.
                return local_writing;
            }

            bool disable_reader() noexcept
            {
                // First set alt to nullptr
                _alt.store(nullptr, std::memory_order_release);
                // Now get the value of writing
                return _writing.load(std::memory_order_acquire);
                // At this point we know one of the following:
                // 1. The write has not committed.
                // 2. The write has committed, and previously saw alt.
                // 3. The write has committed, but never saw alt.
                // 1 will return false (the value of local_writing)
                // 2 & 3 will return true.  The write has committed, so is ready.  This is the value of local_writing.
                // In both cases we can return local_writing.
            }

            bool enable_writer(alt_internal *alt) noexcept
            {
                throw std::logic_error("This channel type has an unguarded writer end");
            }

            bool disable_writer() noexcept
            {
                throw std::logic_error("This channel type has an unguarded writer end");
            }

            bool reader_pending() noexcept
            {
                // Just check if we are in a writing state.
                return _writing.load(std::memory_order_acquire);
            }

            bool writer_pending() noexcept
            {
                throw std::logic_error("This channel type has an unguarded writer end");
            }

            void reader_poison(size_t strength) noexcept
            {
                throw std::logic_error("This channel type has an unguarded writer end");
            }

            void writer_poison(size_t strength) noexcept
            {
                throw std::logic_error("This channel type has an unguarded writer end");
            }
        };

        class atomic_channel_end_mutex : public channel_end_mutex
        {
        private:
            std::atomic_flag _flag = ATOMIC_FLAG_INIT;
        public:
            atomic_channel_end_mutex() = default;

            inline void lock() { while (_flag.test_and_set()); }

            inline void unlock() { _flag.clear(); }
        };

        class barrier_type final : public barrier_internal
        {
        private:
            std::atomic<size_t> _enrolled;
            std::atomic<size_t> _iteration;
            std::atomic<size_t> _syncing;
        public:
            barrier_type()
            : _enrolled(0), _iteration(0), _syncing(0)
            {
            }

            explicit barrier_type(size_t enrolled)
            : _enrolled(enrolled), _iteration(0), _syncing(0)
            {
            }

            void sync() noexcept
            {
                // Get current iteration
                auto iter = _iteration.load(std::memory_order_acquire);
                // Now increment syncing processes, and check if last has synced.
                if (_syncing.fetch_add(1, std::memory_order_acq_rel) == _enrolled.load(std::memory_order_acquire) - 1)
                {
                    // Reset the number of processes syncing
                    _syncing.store(0, std::memory_order_relaxed);
                    // Increment iteration, releasing all spinning processes
                    _iteration.fetch_add(1, std::memory_order_release);
                }
                else
                    while (_iteration.load(std::memory_order_acquire) == iter);
            }

            void enroll() noexcept
            {
                // Increment enrolled and countdown
                _enrolled.fetch_add(1, std::memory_order_release);
            }

            void resign() noexcept
            {
                // First get current iteration - may need it
                auto iter = _iteration.load(std::memory_order_acquire);
                // Now decrement the enrolled count and check if we should notify
                if (_enrolled.fetch_sub(1, std::memory_order_acq_rel) == _syncing.load(std::memory_order_acquire) - 1)
                {
                    // Need to notify waiting processes that no other process will sync
                    _syncing.store(0, std::memory_order_relaxed);
                    _iteration.fetch_add(1, std::memory_order_release);
                }
                // Otherwise we have nothing to do
            }

            void reset(size_t enrolled) noexcept
            {
                assert(_syncing.load(std::memory_order_acquire) == 0);
                // Reset enrolled
                _enrolled.store(enrolled);
            }
        };
    }

    struct atomic_model
    {
        static constexpr concurrency model_type = concurrency::ATOMIC_MODEL;

        using par_type = thread_implementation::parallel_type;
        template<typename T, bool POISONABLE = false>
        using chan_type = atomic_implementation::channel_type<T, POISONABLE>;
        using chan_end_mutex = atomic_implementation::atomic_channel_end_mutex;
        using bar_type = atomic_implementation::barrier_type;
        using alt_type = thread_implementation::alt_type;

        template<typename T, bool POISONABLE = false>
        inline static one2one_chan<T, POISONABLE> make_one2one() noexcept
        {
            channel<T, POISONABLE> c(std::make_shared<chan_type<T, POISONABLE>>());
            return one2one_chan<T, POISONABLE>(c, guarded_chan_in(c), chan_out(c));
        }

        template<typename T, bool POISONABLE = false>
        inline static one2any_chan<T, POISONABLE> make_one2any() noexcept
        {
            channel<T, POISONABLE> c(std::make_shared<chan_type<T, POISONABLE>>());
            return one2any_chan<T, POISONABLE>(c, shared_chan_in(c, std::make_unique<chan_end_mutex>()), chan_out(c));
        }

        template<typename T, bool POISONABLE = false>
        inline static any2one_chan<T, POISONABLE> make_any2one() noexcept
        {
            channel<T, POISONABLE> c(std::make_shared<chan_type<T, POISONABLE>>());
            return any2one_chan<T, POISONABLE>(c, guarded_chan_in(c), shared_chan_out(c, std::make_unique<chan_end_mutex>()));
        }

        template<typename T, bool POISONABLE = false>
        inline static any2any_chan<T, POISONABLE> make_any2any() noexcept
        {
            channel<T, POISONABLE> c(std::make_shared<chan_type<T, POISONABLE>>());
            return any2any_chan<T, POISONABLE>(c, shared_chan_in(c, std::make_unique<chan_end_mutex>()), shared_chan_out(c, std::make_unique<chan_end_mutex>()));
        }

        inline static barrier make_bar(size_t enrolled = 0) { return barrier(std::make_shared<bar_type>(enrolled)); }

        template<class Rep, class Period>
        inline static void sleep(const std::chrono::duration<Rep, Period> &duration) noexcept
        {
            std::this_thread::sleep_for(duration);
        }

        template<class Clock, class Duration>
        inline static void sleep(const std::chrono::time_point<Clock, Duration> &timepoint) noexcept
        {
            std::this_thread::sleep_until(timepoint);
        }
    };
}