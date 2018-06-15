#pragma once

#include <functional>
#include <memory>
#include "concurrency.hpp"

namespace csp
{
    class process;

    struct proc_t
    {
    private:
        mutable std::unique_ptr<process> _proc = nullptr;

    public:
        proc_t() = delete;

        explicit proc_t(std::unique_ptr<process> &&proc)
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

        inline void set_model(concurrency model) const noexcept;

        inline void run() const noexcept;

        inline void operator()() const noexcept { run(); }
    };

    class primitive_builder
    {
    private:
        primitive_builder() = default;
    public:
        virtual ~primitive_builder() = default;

        static proc_t make_par(concurrency conc, const std::vector<proc_t> &procs) noexcept;

        static proc_t make_par(concurrency conc, std::vector<proc_t> &&procs) noexcept;

        template<typename T, bool POISONABLE = false>
        static one2one_chan<T, POISONABLE> make_one2one(concurrency conc) noexcept;

        template<typename T, bool POISONABLE = false>
        static one2any_chan<T, POISONABLE> make_one2any(concurrency conc) noexcept;

        template<typename T, bool POISONABLE = false>
        static any2one_chan<T, POISONABLE> make_any2one(concurrency conc) noexcept;

        template<typename T, bool POISONABLE = false>
        static any2any_chan<T, POISONABLE> make_any2any(concurrency conc) noexcept;

        static barrier make_bar(concurrency conc, size_t enrolled = 0) noexcept;

        static alternative make_alt(concurrency conc, const std::vector<guard> &guards) noexcept;

        static alternative make_alt(concurrency conc, std::vector<guard> &&guards) noexcept;
    };

	class process
	{
	private:
        concurrency _model = concurrency::THREAD_MODEL;
    protected:
	    inline proc_t make_par(const std::vector<proc_t> &procs) const noexcept { return primitive_builder::make_par(_model, procs); }

	    inline proc_t make_par(std::vector<proc_t> &&procs) const noexcept { return primitive_builder::make_par(_model, move(procs)); }

	    inline void par(const std::vector<proc_t> &procs) const noexcept
        {
            auto tmp = primitive_builder::make_par(_model, procs);
            tmp();
        }

        inline void par(std::vector<proc_t> &&procs) const noexcept
        {
            auto tmp = primitive_builder::make_par(_model, move(procs));
            tmp();
        }

        template<typename T, bool POISONABLE = false>
        inline one2one_chan<T, POISONABLE> make_one2one() const noexcept
        {
            return primitive_builder::make_one2one<T, POISONABLE>(_model);
        }

        template<typename T, bool POISONABLE = false>
        inline one2any_chan<T, POISONABLE> make_one2any() const noexcept
        {
            return primitive_builder::make_one2any<T, POISONABLE>(_model);
        }

        template<typename T, bool POISONABLE = false>
        inline any2one_chan<T, POISONABLE> make_any2one() const noexcept
        {
            return primitive_builder::make_any2one<T, POISONABLE>(_model);
        }

        template<typename T, bool POISONABLE = false>
        inline any2any_chan<T, POISONABLE> make_any2any() const noexcept
        {
            return primitive_builder::make_any2any<T, POISONABLE>(_model);
        }

        inline barrier make_bar(size_t enrolled = 0) const noexcept
        {
            return primitive_builder::make_bar(_model, std::move(enrolled));
        }

        inline alternative make_alt(const std::vector<guard> &guards) const noexcept
        {
            return primitive_builder::make_alt(_model, guards);
        }

        inline alternative make_alt(std::vector<guard> &&guards) const noexcept
        {
            return primitive_builder::make_alt(_model, move(guards));
        }

        inline alternative make_alt(std::initializer_list<guard> &&guards) const noexcept
        {
            return primitive_builder::make_alt(_model, std::vector<guard>(move(guards)));
        }

    public:

        inline void set_model(concurrency model) noexcept { _model = model; }

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

    inline void proc_t::run() const noexcept
    {
        if (_proc == nullptr)
            throw std::logic_error("Process has moved");
        _proc->run();
    }

    inline void proc_t::set_model(concurrency model) const noexcept { _proc->set_model(model); }

    template<typename PROCESS, typename... ARGS>
    proc_t make_proc(ARGS&&... args)
    {
        return proc_t(std::make_unique<PROCESS>(args...));
    }
}