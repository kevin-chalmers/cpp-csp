#pragma once

#include <mutex>
#include <condition_variable>

namespace csp
{
    class barrier_internal
    {
    protected:
        barrier_internal() = default;
    public:
        barrier_internal(const barrier_internal&) = default;

        barrier_internal(barrier_internal&&) = default;

        barrier_internal&operator=(const barrier_internal&) = default;

        barrier_internal&operator=(barrier_internal&&) = default;

        virtual ~barrier_internal() = default;

        virtual void sync() noexcept = 0;

        virtual void enroll() noexcept = 0;

        virtual void resign() noexcept = 0;

        virtual void reset(size_t enrolled) noexcept = 0;
    };

	class barrier
	{
    private:
        std::shared_ptr<barrier_internal> _internal = nullptr;
	public:
		explicit barrier(std::shared_ptr<barrier_internal> internal)
        : _internal(std::move(internal))
		{
		}

		barrier(const barrier&) = default;

		barrier(barrier&&) = default;

		~barrier() = default;

		barrier&operator=(const barrier&) = default;

		barrier&operator=(barrier&&) = default;

		inline void sync() const noexcept
        {
            _internal->sync();
        }

        inline void enroll() const noexcept
        {
            _internal->enroll();
        }

        inline void resign() const noexcept
        {
            _internal->resign();
        }

        inline void reset(size_t enrolled) const noexcept
        {
            _internal->reset(std::move(enrolled));
        }

		inline void operator()() const noexcept
		{
			sync();
		}
	};
}