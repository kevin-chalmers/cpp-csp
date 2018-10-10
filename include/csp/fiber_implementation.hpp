#pragma once

#include <memory>
#include <vector>
#include <set>
#include <mutex>
#include <boost/fiber/all.hpp>
#include "channel.hpp"
#include "barrier.hpp"
#include "parallel.hpp"
#include "alternative.hpp"

namespace csp
{
    namespace fiber_implementation
    {
        class fiber_manager
        {
        private:
            thread_local static std::set<std::shared_ptr<boost::fibers::fiber>> _thread_fibers;
            thread_local static std::unique_ptr<boost::fibers::mutex> _thread_fibers_mutex;
        public:
            static void add_to_thread_fibers(std::shared_ptr<boost::fibers::fiber> fiber)
            {
                std::lock_guard<boost::fibers::mutex> lock(*_thread_fibers_mutex);
                _thread_fibers.emplace(fiber);
            }

            static void remove_from_thread_fibers(std::shared_ptr<boost::fibers::fiber> fiber)
            {
                std::lock_guard<boost::fibers::mutex> lock(*_thread_fibers_mutex);
                _thread_fibers.erase(fiber);
            }
        };

        thread_local std::set<std::shared_ptr<boost::fibers::fiber>> fiber_manager::_thread_fibers = std::set<std::shared_ptr<boost::fibers::fiber>>();
        thread_local std::unique_ptr<boost::fibers::mutex> fiber_manager::_thread_fibers_mutex = std::make_unique<boost::fibers::mutex>();

        template<typename T, bool POISONABLE = false>
        class channel_type final : public csp::channel_internal<T, POISONABLE>
        {
        private:
            boost::fibers::mutex _mut;
            boost::fibers::condition_variable _cond;
            std::vector<T> _hold;
            bool _reading = false;
            bool _empty = true;
            alt_internal *_alt = nullptr;
            unsigned int _strength = 0;
        public:
            channel_type()
            {
            }

            void write(T value)
            {
                std::unique_lock<boost::fibers::mutex> lock(_mut);
                if (_strength > 0)
                    throw poison_exception(_strength);
                _hold.push_back(value);
                if (_empty)
                {
                    _empty = false;
                    if (_alt != nullptr)
                        this->schedule(_alt);
                }
                else
                {
                    _empty = true;
                    _cond.notify_one();
                }
                _cond.wait(lock);
                if (_strength > 0)
                    throw poison_exception(_strength);
            }

            template<typename _T = T, IsNotReference<_T>>
            void write(T&& value)
            {
                std::unique_lock<boost::fibers::mutex> lock(_mut);
                if (_strength > 0)
                    throw poison_exception(_strength);
                _hold.push_back(std::move(value));
                if (_empty)
                {
                    _empty = false;
                    if (_alt != nullptr)
                        this->schedule(_alt);
                }
                else
                {
                    _empty = true;
                    _cond.notify_one();
                }
                _cond.wait(lock);
                if (_strength > 0)
                    throw poison_exception(_strength);
            }

            T read()
            {
                std::unique_lock<boost::fibers::mutex> lock(_mut);
                if (_strength > 0)
                    throw poison_exception(_strength);
                if (_empty)
                {
                    _empty = false;
                    _cond.wait(lock);
                }
                else
                    _empty = true;
                auto to_return = std::move(_hold[0]);
                _hold.pop_back();
                _cond.notify_one();
                if (_strength > 0)
                    throw poison_exception(_strength);
                return std::move(to_return);
            }

            T start_read()
            {
                std::unique_lock<boost::fibers::mutex> lock(_mut);
                if (_strength > 0)
                    throw poison_exception(_strength);
                if (_reading)
                    throw std::logic_error("Channel already in extended read");
                if (_empty)
                {
                    _empty = false;
                    _cond.wait(lock);
                }
                else
                    _empty =true;
                _reading = true;
                if (_strength > 0)
                    throw poison_exception(_strength);
                return std::move(_hold[0]);
            }

            void end_read()
            {
                std::unique_lock<boost::fibers::mutex> lock(_mut);
                if (!_reading)
                    throw std::logic_error("Channel not in extended read");
                _hold.pop_back();
                _empty = true;
                _reading = false;
                _cond.notify_one();
            }

            bool enable_reader(alt_internal *alt) noexcept
            {
                std::unique_lock<boost::fibers::mutex> lock(_mut);
                if (_strength > 0)
                    return true;
                if (_empty)
                {
                    _alt = alt;
                    return false;
                }
                else
                    return true;
            }

            bool disable_reader() noexcept
            {
                std::unique_lock<boost::fibers::mutex> lock(_mut);
                _alt = nullptr;
                return !_empty || (_strength > 0);
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
                return !_empty || (_strength > 0);
            }

            bool writer_pending() noexcept
            {
                throw std::logic_error("This channel type has an unguarded writer end");
            }

            void reader_poison(size_t strength) noexcept
            {
                std::unique_lock<boost::fibers::mutex> lock(_mut);
                _strength = strength;
                _cond.notify_all();
            }

            void writer_poison(size_t strength) noexcept
            {
                std::unique_lock<boost::fibers::mutex> lock(_mut);
                _strength = strength;
                _cond.notify_all();
                if (_alt != nullptr)
                    this->schedule(_alt);
            }
        };

        class fiber_channel_end_mutex final : public channel_end_mutex
        {
        private:
            boost::fibers::mutex _mut;
        public:
            fiber_channel_end_mutex() = default;

            inline void lock() { _mut.lock(); }

            inline void unlock() { _mut.unlock(); }
        };

        class barrier_type final : public barrier_internal
        {
        private:
            size_t _enrolled = 0;
            size_t _count_down = 0;
            boost::fibers::mutex _mut;
            boost::fibers::condition_variable _cond;
        public:
            barrier_type()
            {
            }

            explicit barrier_type(size_t enrolled)
            : _enrolled(enrolled), _count_down(enrolled)
            {

            }

            void sync() noexcept
            {
                std::unique_lock<boost::fibers::mutex> lock(_mut);
                --_count_down;
                if (_count_down > 0)
                    _cond.wait(lock);
                else
                {
                    _count_down = _enrolled;
                    _cond.notify_all();
                }
            }

            void enroll() noexcept
            {
                std::lock_guard<boost::fibers::mutex> lock(_mut);
                ++_enrolled;
                ++_count_down;
            }

            void resign() noexcept
            {
                std::lock_guard<boost::fibers::mutex> lock(_mut);
                --_enrolled;
                --_count_down;
                if (_count_down == 0)
                {
                    _count_down = _enrolled;
                    _cond.notify_all();
                }
            }

            void reset(size_t enrolled) noexcept
            {
                std::lock_guard<boost::fibers::mutex> lock(_mut);
                _enrolled = enrolled;
                _count_down = enrolled;
            }
        };

        class parallel_type;

        class thread_type
        {
            friend class parallel_type;

        private:
            proc_t *_process = nullptr;
            std::shared_ptr<boost::fibers::fiber> _fiber = nullptr;
            barrier _bar;
            barrier _park = barrier(std::make_shared<barrier_type>(2));
            bool _running = true;
        public:
            thread_type(proc_t *proc, barrier &bar)
            : _process(proc), _bar(bar)
            {

            }

            ~thread_type()
            {
                fiber_manager::remove_from_thread_fibers(_fiber);
            }

            void reset(proc_t *proc, barrier &bar) noexcept
            {
                _process = proc;
                _bar = bar;
                _running = true;
            }

            void terminate() noexcept
            {
                _running = false;
                _park();
            }

            void release() noexcept
            {
                _park();
            }

            void run() noexcept
            {
                while (_running)
                {
                    _process->run();
                    _bar();
                    _park();
                }
                fiber_manager::remove_from_thread_fibers(_fiber);
            }

            void start() noexcept
            {
                _fiber = std::make_shared<boost::fibers::fiber>(&thread_type::run, this);
                fiber_manager::add_to_thread_fibers(_fiber);
                _running = true;
            }
        };

        class parallel_type
        {
            boost::fibers::mutex _mut;
            std::vector<proc_t> _processes;
            std::vector<std::shared_ptr<thread_type>> _fibers;
            barrier _bar = barrier(std::make_shared<barrier_type>(0));
            bool _process_changed = true;
        private:
            void release_all_fibers() noexcept
            {
                std::lock_guard<boost::fibers::mutex> lock(_mut);
                for (auto &f : _fibers)
                {
                    f->terminate();
                    f->_fiber->join();
                }
                _process_changed = true;
                _fibers.clear();
            }
        public:
            parallel_type()
            {
            }

            explicit parallel_type(std::initializer_list<proc_t> &&procs)
            : _processes(procs)
            {

            }

            explicit parallel_type(const std::vector<proc_t> &procs)
            {
                _processes = procs;
            }

            template<typename RanIt>
            parallel_type(RanIt begin, RanIt end)
            {
                static_assert(std::iterator_traits<RanIt>::value_type == typeid(proc_t), "par only takes collections of process objects");
                _processes = std::vector<proc_t>(begin, end);
            }

            ~parallel_type()
            {
                release_all_fibers();
            }

            void run() noexcept
            {
                bool empty_run = true;
                proc_t *my_process;
                std::unique_lock<boost::fibers::mutex> lock(_mut);
                if (_processes.size() > 0)
                {
                    empty_run = false;
                    my_process = &_processes[_processes.size() - 1];
                    if (_process_changed)
                    {
                        _bar.reset(static_cast<unsigned int>(_processes.size()));
                        if (_fibers.size() < _processes.size() - 1)
                        {
                            for (size_t i = 0; i < _fibers.size(); ++i)
                            {
                                _fibers[i]->reset(&_processes[i], _bar);
                                _fibers[i]->release();
                            }
                            for (size_t i = static_cast<unsigned int>(_fibers.size()); i < _processes.size() - 1; ++i)
                            {
                                _fibers.push_back(std::shared_ptr<thread_type>(new thread_type(&_processes[i], _bar)));
                                _fibers[i]->start();
                            }
                        }
                        else
                        {
                            _fibers.resize(_processes.size() - 1);
                            for (size_t i = 0; i < _processes.size() - 1; ++i)
                            {
                                _fibers[i]->reset(&_processes[i], _bar);
                                _fibers[i]->release();
                            }
                        }
                        _process_changed = false;
                    }
                    else
                    {
                        for (size_t i = 0; i < _processes.size() - 1; ++i)
                            _fibers[i]->release();
                    }
                }
                if (!empty_run)
                {
                    my_process->run();
                    _bar();
                }
            }
        };

        class fiber_alt_bar_coord
        {
        private:
            thread_local static size_t _active;
            thread_local static std::unique_ptr<boost::fibers::mutex> _active_lock;
            thread_local static std::unique_ptr<boost::fibers::condition_variable> _cond;
        public:
            static void start_enable()
            {
                std::unique_lock<boost::fibers::mutex> lock(*_active_lock);
                while (_active > 0)
                    _cond->wait(lock);
                if (_active != 0)
                    throw std::runtime_error("alting_barrier enable sequence starting with active count not equal to zero: " + std::to_string(_active));
                _active = 1;
            }

            static void finish_enable()
            {
                std::unique_lock<boost::fibers::mutex> lock(*_active_lock);
                if (_active != 1)
                    throw std::runtime_error("alting barrier enable sequence finished with active count not equal to one: " + std::to_string(_active));
                _active = 0;
                _cond->notify_one();
            }

            static void start_disable(size_t n)
            {
                std::unique_lock<boost::fibers::mutex> lock(*_active_lock);
                if (_active != 1)
                    throw std::runtime_error("Completed alting barrier found in alt sequence with active count not equal to one: " + std::to_string(_active));
                _active = n;
            }

            static void finish_disable()
            {
                std::unique_lock<boost::fibers::mutex> lock(*_active_lock);
                if (_active < 1)
                    throw std::runtime_error("alting barrier disable sequence finished with active count less than one: " + std::to_string(_active));
                --_active;
                if (_active == 0)
                    _cond->notify_one();
            }
        };

        thread_local size_t fiber_alt_bar_coord::_active = 0;
        thread_local std::unique_ptr<boost::fibers::mutex> fiber_alt_bar_coord::_active_lock = std::make_unique<boost::fibers::mutex>();
        thread_local std::unique_ptr<boost::fibers::condition_variable> fiber_alt_bar_coord::_cond = std::make_unique<boost::fibers::condition_variable>();

        class alt_type final : public alt_internal
        {
        private:
            boost::fibers::mutex _mut;
            boost::fibers::condition_variable _cond;

            void enable_guards() noexcept
            {
                if (_barrier_present)
                    fiber_alt_bar_coord::finish_enable();
                _barrier_selected = 0;
                for (_enable_index = _next; _enable_index < _guards.size(); ++_enable_index)
                {
                    if (_guards[_enable_index].enable(this))
                    {
                        _selected = _enable_index;
                        _state = STATE::READY;
                        if (_barrier_trigger)
                        {
                            _barrier_selected = _selected;
                            _barrier_ready = true;
                            _barrier_trigger = false;
                        }
                        else if (_barrier_present)
                            fiber_alt_bar_coord::finish_enable();
                        _none_selected = false;
                        return;
                    }
                }
                for (_enable_index = 0; _enable_index < _next; ++_enable_index)
                {
                    if (_guards[_enable_index].enable(this))
                    {
                        _selected = _enable_index;
                        _state = STATE::READY;
                        if (_barrier_trigger)
                        {
                            _barrier_selected = _selected;
                            _barrier_ready = true;
                            _barrier_trigger = false;
                        }
                        else if (_barrier_present)
                            fiber_alt_bar_coord::finish_enable();
                        _none_selected = false;
                        return;
                    }
                }
                _none_selected = true;
                if (_barrier_present)
                    fiber_alt_bar_coord::finish_enable();
            }

            void enable_guards(const std::vector<bool> &pre_cond) noexcept
            {
                if (_barrier_present)
                    fiber_alt_bar_coord::finish_enable();
                _barrier_selected = 0;
                for (_enable_index = _next; _enable_index < _guards.size(); ++_enable_index)
                {
                    if (pre_cond[_enable_index] && _guards[_enable_index].enable(this))
                    {
                        _selected = _enable_index;
                        _state = STATE::READY;
                        if (_barrier_trigger)
                        {
                            _barrier_selected = _selected;
                            _barrier_ready = true;
                            _barrier_trigger = false;
                        }
                        else if (_barrier_present)
                            fiber_alt_bar_coord::finish_enable();
                        _none_selected = false;
                        return;
                    }
                }
                for (_enable_index = 0; _enable_index < _next; ++_enable_index)
                {
                    if (pre_cond[_enable_index] && _guards[_enable_index].enable(this))
                    {
                        _selected = _enable_index;
                        _state = STATE::READY;
                        if (_barrier_trigger)
                        {
                            _barrier_selected = _selected;
                            _barrier_ready = true;
                            _barrier_trigger = false;
                        }
                        else if (_barrier_present)
                            fiber_alt_bar_coord::finish_enable();
                        _none_selected = false;
                        return;
                    }
                }
                _none_selected = true;
                if (_barrier_present)
                    fiber_alt_bar_coord::finish_enable();
            }

            void disable_guards() noexcept
            {
                if (_selected != _next)
                {
                    long long start_index = (_none_selected ? static_cast<long long>(_next) - 1 : static_cast<long long>(_selected) - 1);
                    if (start_index < static_cast<long long>(_next))
                    {
                        for (auto i = start_index; i >= 0; --i)
                        {
                            if (_guards[i].disable())
                            {
                                _none_selected = false;
                                _selected = i;
                                if (_barrier_trigger)
                                {
                                    _barrier_selected = _selected;
                                    _barrier_trigger = false;
                                }
                            }
                        }
                        start_index = static_cast<long long>(_guards.size()) - 1;
                    }
                    for (auto i = start_index; i >= static_cast<long long>(_next); --i)
                    {
                        if (_guards[i].disable())
                        {
                            _none_selected = false;
                            _selected = i;
                            if (_barrier_trigger)
                            {
                                _barrier_selected = _selected;
                                _barrier_trigger = false;
                            }
                        }
                    }
                    if (_none_selected)
                        _selected = _timer_index;
                }
                if (_barrier_ready)
                {
                    _selected = _barrier_selected;
                    fiber_alt_bar_coord::finish_disable();
                }
            }

            void disable_guards(const std::vector<bool> &pre_cond) noexcept
            {
                if (_selected != _next)
                {
                    long long start_index = (_none_selected ? static_cast<long long>(_next) - 1 : static_cast<long long>(_selected) - 1);
                    if (start_index < static_cast<long long>(_next))
                    {
                        for (auto i = start_index; i >= 0; --i)
                        {
                            if (pre_cond[i] && _guards[i].disable())
                            {
                                _none_selected = false;
                                _selected = i;
                                if (_barrier_trigger)
                                {
                                    _barrier_selected = _selected;
                                    _barrier_trigger = false;
                                }
                            }
                        }
                        start_index = static_cast<long long>(_guards.size()) - 1;
                    }
                    for (auto i = start_index; i >= static_cast<long long>(_next); --i)
                    {
                        if (pre_cond[i] && _guards[i].disable())
                        {
                            _none_selected = false;
                            _selected = i;
                            if (_barrier_trigger)
                            {
                                _barrier_selected = _selected;
                                _barrier_trigger = false;
                            }
                        }
                    }
                    if (_none_selected)
                        _selected = _timer_index;
                }
                if (_barrier_ready)
                {
                    _selected = _barrier_selected;
                    fiber_alt_bar_coord::finish_disable();
                }
            }
        protected:
            size_t do_select() noexcept
            {
                _state = STATE::ENABLING;
                enable_guards();
                {
                    std::unique_lock<boost::fibers::mutex> lock(_mut);
                    if (_state == STATE::ENABLING)
                    {
                        _state = STATE::WAITING;
                        if (_timeout)
                            _cond.wait_until(lock, _time);
                        else
                            _cond.wait(lock);
                        _state = STATE::READY;
                    }
                }
                disable_guards();
                _state = STATE::INACTIVE;
                _timeout = false;
                _none_selected = true;
                _barrier_ready = false;
                return _selected;
            }

            size_t do_select(const std::vector<bool> &pre_conds) noexcept
            {
                _state = STATE::ENABLING;
                enable_guards(pre_conds);
                {
                    std::unique_lock<boost::fibers::mutex> lock(_mut);
                    if (_state == STATE::ENABLING)
                    {
                        _state = STATE::WAITING;
                        if (_timeout)
                            _cond.wait_until(lock, _time);
                        else
                            _cond.wait(lock);
                        _state = STATE::READY;
                    }
                }
                disable_guards(pre_conds);
                _state = STATE::INACTIVE;
                _timeout = false;
                _none_selected = true;
                _barrier_ready = false;
                return _selected;
            }

        public:
            alt_type() = default;

            alt_type(const std::vector<guard> &guards)
            : alt_internal(guards)
            {
            }

            alt_type(std::vector<guard> &&guards)
            : alt_internal(move(guards))
            {
            }

            alt_type(const alt_type&) = delete;

            alt_type(alt_type&&) = default;

            ~alt_type() = default;

            alt_type&operator=(const alt_type&) = delete;

            alt_type&operator=(alt_type&&) = default;

            void schedule() noexcept
            {
                std::unique_lock<boost::fibers::mutex> lock(_mut);

                switch (_state)
                {
                    case STATE::ENABLING:
                        _state = STATE::READY;
                        break;
                    case STATE::WAITING:
                        _state = STATE::READY;
                        _cond.notify_one();
                        break;
                    default:
                        break;
                }
            }
        };
    }

    struct fiber_model
    {
        static constexpr concurrency model_type = concurrency::FIBER_MODEL;

        using par_type = fiber_implementation::parallel_type;
        template<typename T, bool POISONABLE>
        using chan_type = fiber_implementation::channel_type<T, POISONABLE>;
        using chan_end_mutex = fiber_implementation::fiber_channel_end_mutex;
        using bar_type = fiber_implementation::barrier_type;
        using alt_type = fiber_implementation::alt_type;

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
            boost::this_fiber::sleep_for(duration);
        }

        template<class Clock, class Duration>
        inline static void sleep(const std::chrono::time_point<Clock, Duration> &timepoint) noexcept
        {
            boost::this_fiber::sleep_until(timepoint);
        }
    };
}