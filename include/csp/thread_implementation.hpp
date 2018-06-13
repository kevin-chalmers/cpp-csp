#pragma once

#include <memory>
#include <vector>
#include <set>
#include <functional>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <cassert>
#include "channel.hpp"
#include "barrier.hpp"
#include "parallel.hpp"
#include "alternative.hpp"

namespace csp
{
	namespace thread_implementation
	{
		class thread_manager
		{
		private:
			static std::set<std::shared_ptr<std::thread>> _all_threads;
			static std::unique_ptr<std::mutex> _all_threads_lock;
		public:
			static void add_to_all_threads(std::shared_ptr<std::thread> thread) noexcept
			{
				std::lock_guard<std::mutex> lock(*_all_threads_lock);
				_all_threads.emplace(thread);
			}

			static void remove_from_all_threads(std::shared_ptr<std::thread> thread) noexcept
			{
				std::lock_guard<std::mutex> lock(*_all_threads_lock);
				_all_threads.erase(thread);
			}
		};

		std::set<std::shared_ptr<std::thread>> thread_manager::_all_threads = std::set<std::shared_ptr<std::thread>>();
		std::unique_ptr<std::mutex> thread_manager::_all_threads_lock = std::make_unique<std::mutex>();

		template<typename T, bool POISONABLE = false>
		class channel_type final : public csp::channel_internal<T, POISONABLE>
		{
		private:
            std::mutex _mut;
            std::condition_variable _cond;
            std::vector<T> _hold = std::vector<T>(0);
            bool _reading = false;
            bool _empty = true;
            size_t _strength = 0;
		public:
			channel_type()
			{
			}

			void write(T value)
			{
				std::unique_lock<std::mutex> lock(_mut);
				if (_strength > 0)
					throw poison_exception(_strength);
				_hold.push_back(std::move(value));
				if (_empty)
				{
					_empty = false;
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

            void write(T&& value)
            {
                std::unique_lock<std::mutex> lock(_mut);
                if (_strength > 0)
                    throw poison_exception(_strength);
                _hold.push_back(value);
                if (_empty)
                {
                    _empty = false;
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
				std::unique_lock<std::mutex> lock(_mut);
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
				return T();
			}

			void end_read()
			{

			}

            bool enable_reader(alt_internal const* alt) noexcept
            {
                return true;
            }

            bool disable_reader() noexcept
            {
                return true;
            }

            bool enable_writer(alt_internal const* alt) noexcept
            {
                return true;
            }

            bool disable_writer() noexcept
            {
                return true;
            }

			bool reader_pending() noexcept
			{
				return false;
			}

			bool writer_pending() noexcept
            {
                return false;
            }

			void reader_poison(size_t strength) noexcept
			{

			}

			void writer_poison(size_t strength) noexcept
			{

			}
		};

		class barrier_type : public barrier_internal
		{
		private:
            size_t _enrolled = 0;
            size_t _count_down = 0;
            std::mutex _mut;
            std::condition_variable _cond;

		public:
			barrier_type()
			{
			}

			explicit barrier_type(size_t size)
			{
				_enrolled = size;
				_count_down = size;
			}

			void sync() noexcept
			{
				std::unique_lock<std::mutex> lock(_mut);
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
				std::lock_guard<std::mutex> lock(_mut);
				++_enrolled;
				++_count_down;
			}

			void resign() noexcept
			{
				std::lock_guard<std::mutex> lock(_mut);
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
			    assert(_enrolled == _count_down);
				std::lock_guard<std::mutex> lock(_mut);
				_enrolled = enrolled;
				_count_down = enrolled;
			}
		};

		class thread_type
		{
		public:
			proc_t *_process;
			std::shared_ptr<std::thread> _thread = nullptr;
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
				thread_manager::remove_from_all_threads(_thread);
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
				_park.sync();
			}

			void release() noexcept
			{
				_park.sync();
			}

			void run() noexcept
			{
				while (_running)
				{
                    _process->run();
					_bar.sync();
					_park.sync();
				}
			}

			void start() noexcept
			{
				_thread = std::make_shared<std::thread>(&thread_type::run, this);
				thread_manager::add_to_all_threads(_thread);
				_running = true;
			}
		};

		class parallel_type
		{
		private:
			std::mutex _mut;
			std::vector<proc_t> _processes;
			std::vector<std::shared_ptr<thread_type>> _threads;
			barrier _bar = barrier(std::make_shared<barrier_type>(0));
			bool _processes_changed = true;

			void release_all_threads() noexcept
			{
				std::lock_guard<std::mutex> lock(_mut);
				for (auto &t : _threads)
				{
					t->terminate();
					t->_thread->join();
				}
				_processes_changed = true;
				_threads.clear();
			}

		public:
		    parallel_type()
            {
            }

            explicit parallel_type(std::initializer_list<proc_t> &&procs)
            : _processes(procs)
            {

            }

			explicit parallel_type(std::vector<proc_t> &procs)
			{
				std::swap(_processes, procs);
			}

			template<typename RanIt>
			parallel_type(RanIt begin, RanIt end)
			{
				static_assert(std::iterator_traits<RanIt>::value_type == typeid(proc_t), "par only takes collections of process objects");
				_processes = std::vector<proc_t>(begin, end);
			}

			~parallel_type()
            {
                release_all_threads();
            }

			void run() noexcept
			{
				bool empty_run = true;
				proc_t *my_process;
				std::lock_guard<std::mutex> lock(_mut);
				if (_processes.size() > 0)
				{
					empty_run = false;
					my_process = &_processes[_processes.size() - 1];
					if (_processes_changed)
					{
						_bar.reset(_processes.size());
						if (_threads.size() < _processes.size() - 1)
						{
							for (size_t i = 0; i < _threads.size(); ++i)
							{
								_threads[i]->reset(&_processes[i], _bar);
								_threads[i]->release();
							}
							for (size_t i = _threads.size(); i < _processes.size() - 1; ++i)
							{
								_threads.push_back(std::make_shared<thread_type>(&_processes[i], _bar));
								_threads[i]->start();
							}
						}
						else
						{
							for (size_t i = _threads.size() - 1; i >= _processes.size() - 1; --i)
							{
								_threads[i]->_running = false;
								_threads[i]->release();
							}
							_threads.resize(_processes.size() - 1);
							for (size_t i = 0; i < _processes.size() - 1; ++i)
							{
								_threads[i]->reset(&_processes[i], _bar);
								_threads[i]->release();
							}
						}
						_processes_changed = false;
					}
					else
					{
						for (auto &t : _threads)
							t->release();
					}
				}
				if (!empty_run)
				{
                    my_process->run();
					_bar.sync();
				}
			}
		};

		class mutex_alt_bar_coord
        {
        private:
            static size_t _active;

            static std::unique_ptr<std::mutex> _active_lock;

            static std::unique_ptr<std::condition_variable> _cond;
        public:
            static void start_enable()
            {
                std::unique_lock<std::mutex> lock(*_active_lock);
                while (_active > 0)
                    _cond->wait(lock);
                if (_active != 0)
                    throw std::runtime_error("alting_barrier enable sequence starting with active count not equal to zero: " + std::to_string(_active));
                _active = 1;
            }

            static void finish_enable()
            {
                std::unique_lock<std::mutex> lock(*_active_lock);
                if (_active != 1)
                    throw std::runtime_error("alting barrier enable sequence finished with active count not equal to one: " + std::to_string(_active));
                _active = 0;
                _cond->notify_one();
            }

            static void start_disable(size_t n)
            {
                std::unique_lock<std::mutex> lock(*_active_lock);
                if (_active != 1)
                    throw std::runtime_error("Completed alting barrier found in alt sequence with active count not equal to one: " + std::to_string(_active));
                _active = n;
            }

            static void finish_disable()
            {
                std::unique_lock<std::mutex> lock(*_active_lock);
                if (_active < 1)
                    throw std::runtime_error("alting barrier disable sequence finished with active count less than one: " + std::to_string(_active));
                --_active;
                if (_active == 0)
                    _cond->notify_one();
            }
        };

		size_t mutex_alt_bar_coord::_active = 0;
		std::unique_ptr<std::mutex> mutex_alt_bar_coord::_active_lock = std::make_unique<std::mutex>();
		std::unique_ptr<std::condition_variable> mutex_alt_bar_coord::_cond = std::make_unique<std::condition_variable>();

		class alt_type final : public alt_internal
		{
		private:
		    std::mutex _mut;

		    std::condition_variable _cond;
		protected:
		    void enable_guards() noexcept
            {
                if (_barrier_present)
                    mutex_alt_bar_coord::finish_enable();
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
                            mutex_alt_bar_coord::finish_enable();
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
                            mutex_alt_bar_coord::finish_enable();
                        _none_selected = false;
                        return;
                    }
                }
                _none_selected = true;
                if (_barrier_present)
                    mutex_alt_bar_coord::finish_enable();
            }

            void enable_guards(const std::vector<bool> &pre_cond) noexcept
            {
                if (_barrier_present)
                    mutex_alt_bar_coord::finish_enable();
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
                            mutex_alt_bar_coord::finish_enable();
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
                            mutex_alt_bar_coord::finish_enable();
                        _none_selected = false;
                        return;
                    }
                }
                _none_selected = true;
                if (_barrier_present)
                    mutex_alt_bar_coord::finish_enable();
            }

            void disable_guards() noexcept
            {
                if (_selected != _next)
                {
                    size_t start_index = (_none_selected ? _next - 1 : _selected - 1);
                    if (_selected < _next)
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
                        start_index = _guards.size() - 1;
                    }
                    for (auto i = start_index; i >= _next; --i)
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
                    mutex_alt_bar_coord::finish_disable();
                }
            }

            void disable_guards(const std::vector<bool> &pre_cond) noexcept
            {
                if (_selected != _next)
                {
                    size_t start_index = (_none_selected ? _next - 1 : _selected - 1);
                    if (_selected < _next)
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
                        start_index = _guards.size() - 1;
                    }
                    for (auto i = start_index; i >= _next; --i)
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
                    mutex_alt_bar_coord::finish_disable();
                }
            }

            size_t do_select() noexcept
            {
                _state = STATE::ENABLING;
                enable_guards();
                {
                    std::unique_lock<std::mutex> lock(_mut);
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
                    std::unique_lock<std::mutex> lock(_mut);
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
                std::unique_lock<std::mutex> lock(_mut);

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
	};

	struct thread_model
    {
        static constexpr concurrency model_type = concurrency::THREAD_MODEL;

        using par_type = thread_implementation::parallel_type;
        template<typename T, bool POISONABLE>
        using chan_type = thread_implementation::channel_type<T, POISONABLE>;
        using bar_type = thread_implementation::barrier_type;
        using alt_type = thread_implementation::alt_type;

        template<typename T, bool POISONABLE = false>
        inline static one2one_chan<T, POISONABLE> make_one2one()
        {
            return one2one_chan<T, POISONABLE>(channel<T, POISONABLE>(std::make_shared<thread_implementation::channel_type<T, POISONABLE>>()));
        }

        inline static barrier make_bar(size_t enrolled = 0) { return barrier(std::make_shared<thread_implementation::barrier_type>(enrolled)); }
    };
}