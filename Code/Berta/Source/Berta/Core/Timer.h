/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#ifndef BT_TIMER_HEADER
#define BT_TIMER_HEADER

#include <chrono>
#include <thread>
#include <functional>
#include <atomic>
#include <condition_variable>
#include <mutex>
#include "Berta/Core/Event.h"

namespace Berta
{
	struct ArgTimer
	{
	};
	struct Window;

	class Timer
	{
	public:
		Timer(Window* owner);
		Timer();
		~Timer();

		void Start();
		void Stop();

		void SetOwner(Window* owner) { m_owner = owner; }
		void SetInterval(std::chrono::milliseconds milliseconds);
		void SetInterval(uint32_t milliseconds);
		void Connect(std::function<void(const ArgTimer&)> callback)
		{
			m_tick.Connect(callback);
		}

		bool IsRunning() const { return m_isRunning.load(); }
	private:
		void Run();

		std::mutex m_conditionMutex;
		std::condition_variable m_conditionVariable;
		std::atomic_bool m_isRunning{ false };
		std::atomic<std::chrono::milliseconds> m_interval{ std::chrono::milliseconds{ 1000 } };
		
		std::thread m_timerThread;
		Event<ArgTimer> m_tick;
		Window* m_owner{ nullptr };
	};
}

#endif