#pragma once

#include <vector>
#include <memory>
#include <chrono>
#include <cassert>

namespace csp
{
    class alt_internal;

    class alternative;

    class guard_internal
    {
    protected:
        guard_internal() = default;

        void schedule(alt_internal *const) const noexcept;

        void set_timeout(alt_internal *const, const std::chrono::system_clock::time_point&) const noexcept;

        void set_barrier_trigger(alt_internal *const) const noexcept;

    public:
        guard_internal(const guard_internal&) = default;

        guard_internal(guard_internal&&) = default;

        virtual ~guard_internal() = default;

        guard_internal&operator=(const guard_internal&) = default;

        guard_internal&operator=(guard_internal&&) = default;

        virtual bool enable(alt_internal const*) noexcept = 0;

        virtual bool disable() noexcept = 0;
    };

    class guard
    {
        friend class alt_internal;
    protected:
        guard() = default;

        std::shared_ptr<guard_internal> _guard_internal = nullptr;
    public:
        guard(const guard&) = default;

        guard(guard&&) = default;

        virtual ~guard() = default;

        guard&operator=(const guard&) = default;

        guard&operator=(guard&&) = default;

        bool enable(alt_internal const*) const noexcept;

        bool disable() const noexcept;
    };

    class multiway_sync
    {
    public:
        virtual ~multiway_sync() = default;
    };

    class alt_internal
    {
        friend class guard_internal;
    protected:
        enum class STATE
        {
            ENABLING    = 0,
            WAITING     = 1,
            READY       = 2,
            INACTIVE    = 3
        };

        STATE _state = STATE::INACTIVE;

        std::vector<guard> _guards;

        size_t _next = 0;

        size_t _selected = 0;

        size_t _barrier_selected = 0;

        bool _none_selected = true;

        bool _barrier_present = false;

        bool _barrier_trigger = false;

        bool _barrier_ready = false;

        size_t _enable_index = 0;

        bool _timeout = false;

        std::chrono::system_clock::time_point _time;

        size_t _timer_index = 0;

        alt_internal() = default;

        virtual size_t do_select() noexcept = 0;

        virtual size_t do_select(const std::vector<bool> &pre_conditions) noexcept = 0;

    public:
        explicit alt_internal(const std::vector<guard> &guards)
        : _guards(guards)
        {
            for (auto &g : _guards)
            {
                if (dynamic_cast<multiway_sync*>(g._guard_internal.get()))
                {
                    _barrier_present = true;
                    return;
                }
            }
        }

        explicit alt_internal(std::vector<guard> &&guards)
        : _guards(move(guards))
        {
            for (auto &g : _guards)
            {
                if (dynamic_cast<multiway_sync*>(g._guard_internal.get()))
                {
                    _barrier_present = true;
                    return;
                }
            }
        }

        alt_internal(const alt_internal&) = default;

        alt_internal(alt_internal&&) = default;

        virtual ~alt_internal() = default;

        alt_internal&operator=(const alt_internal&) = default;

        alt_internal&operator=(alt_internal&&) = default;

        inline size_t fair_select() noexcept
        {
            size_t to_return = this->do_select();
            _next = (_next == _guards.size()) ? 0 : to_return + 1;
            return to_return;
        }

        inline size_t fair_select(const std::vector<bool> &pre_cond) noexcept
        {
            assert(pre_cond.size() == _guards.size());
            size_t to_return = this->do_select(pre_cond);
            _next = (_next == _guards.size()) ? 0 : to_return + 1;
            return to_return;
        }

        inline size_t fair_select(std::initializer_list<bool> &&pre_cond) noexcept
        {
            return fair_select(std::vector<bool>(move(pre_cond)));
        }

        inline size_t select() noexcept
        {
            return fair_select();
        }

        inline size_t select(const std::vector<bool> &pre_cond) noexcept
        {
            return fair_select(pre_cond);
        }

        inline size_t select(std::initializer_list<bool> &&pre_cond) noexcept
        {
            return fair_select(move(pre_cond));
        }

        inline size_t pri_select() noexcept
        {
            _next = 0;
            return this->do_select();
        }

        inline size_t pri_select(const std::vector<bool> &pre_cond) noexcept
        {
            assert(pre_cond.size() == _guards.size());
            _next = 0;
            return this->do_select(pre_cond);
        }

        inline size_t pri_select(std::initializer_list<bool> &&pre_cond) noexcept
        {
            return pri_select(std::vector<bool>(move(pre_cond)));
        }

        void set_barrier_trigger() noexcept
        {
            _barrier_trigger = 0;
        }

        void set_timeout(const std::chrono::system_clock::time_point &time) noexcept
        {
            if (_timeout)
            {
                if (time < _time)
                {
                    _time = time;
                    _timer_index = _enable_index;
                }
            }
            else
            {
                _timeout = true;
                _time = time;
                _timer_index = _enable_index;
            }
        }

        virtual void schedule() noexcept = 0;
    };

    class alternative
    {
        friend class guard;
    private:
        std::shared_ptr<alt_internal> _internal = nullptr;

    public:
        explicit alternative(std::shared_ptr<alt_internal> internal)
        : _internal(internal)
        {
        }

        alternative(const alternative&) = default;

        alternative(alternative&&) = default;

        ~alternative() = default;

        alternative&operator=(const alternative&) = default;

        alternative&operator=(alternative&&) = default;

        inline size_t select() const noexcept { return _internal->select(); }

        inline size_t select(const std::vector<bool> &pre_cond) const noexcept { return _internal->select(pre_cond); }

        inline size_t select(std::initializer_list<bool> &&pre_cond) const noexcept { return _internal->select(move(pre_cond)); }

        inline size_t fair_select() const noexcept { return _internal->fair_select(); }

        inline size_t fair_select(const std::vector<bool> &pre_cond) const noexcept { return _internal->fair_select(pre_cond); }

        inline size_t fair_select(std::initializer_list<bool> &&pre_cond) const noexcept { return _internal->fair_select(move(pre_cond)); }

        inline size_t pri_select() const noexcept { return _internal->pri_select(); }

        inline size_t pri_select(const std::vector<bool> &&pre_cond) const noexcept { return _internal->pri_select(pre_cond); }

        inline size_t pri_select(std::initializer_list<bool> &&pre_cond) const noexcept { return _internal->pri_select(move(pre_cond)); }

        inline size_t operator()() const noexcept { return _internal->select(); }

        inline size_t operator()(const std::vector<bool> &pre_cond) const noexcept { return _internal->select(pre_cond); }

        inline size_t operator()(std::initializer_list<bool> &&pre_cond) const noexcept { return _internal->select(move(pre_cond)); }
    };

    bool guard::enable(alt_internal const* alt) const noexcept { return _guard_internal->enable(alt); }

    bool guard::disable() const noexcept { return _guard_internal->disable(); }

    void guard_internal::schedule(alt_internal *const alt) const noexcept
    {
        alt->schedule();
    }

    void guard_internal::set_timeout(alt_internal *const alt, const std::chrono::system_clock::time_point &time) const noexcept
    {
        alt->set_timeout(time);
    }

    void guard_internal::set_barrier_trigger(alt_internal *const alt) const noexcept
    {
        alt->set_barrier_trigger();
    }

}