#pragma once

#include <functional>
#include <memory>
#include "concurrency_model.hpp"

namespace csp
{
	class process
	{
	private:
        concurrency_model_t _model = concurrency_model_t(concurrency::THREAD_MODEL);
	public:

		virtual ~process() = default;

        inline void set_model(concurrency model) noexcept { _model = concurrency_model_t(model); }

		template<typename T, bool POISONABLE = false>
		inline channel<T, POISONABLE> make_chan()
		{
			return _model.make_chan<T, POISONABLE>();
		}

		virtual void run() noexcept = 0;

		inline void operator()() noexcept { this->run(); }
	};

	class process_function final : public process
    {
    private:
        std::function<void()> _func;
    public:
        process_function(std::function<void()> func)
        : _func(std::move(func))
        {
        }

        void set_func(std::function<void()> func) noexcept
        {
            _func = func;
        }

        inline void run() noexcept
        {
            _func();
        }
    };

	struct proc_t
    {
    private:
        mutable std::unique_ptr<process> _proc = nullptr;
    public:
        proc_t() = delete;

        proc_t(std::unique_ptr<process> &&proc)
        : _proc(std::move(proc))
        {
        }

        proc_t(const proc_t &other)
        {
            _proc.swap(other._proc);
        }

        proc_t(proc_t&&) = default;

        proc_t&operator=(const proc_t &other)
        {
            other._proc.swap(_proc);
            return *this;
        }

        proc_t&operator=(proc_t&&) = default;

        ~proc_t() = default;

        inline void set_model(concurrency model) noexcept { _proc->set_model(model); }

        inline void run() const noexcept
        {
            if (_proc == nullptr)
                throw std::logic_error("Process has moved");
            _proc->run();
        }
    };

    template<typename PROCESS, typename... ARGS>
    proc_t make_proc(ARGS&&... args)
    {
        return proc_t(std::make_unique<PROCESS>(args...));
    }
}