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
#include "Berta/Core/Event.h"

namespace Berta
{
	struct ArgTimer
	{
	};

	class Timer
	{
	public:
		Timer();
		~Timer();

		void Start();
		void Stop();

		void SetInterval(std::chrono::milliseconds milliseconds) { m_interval = milliseconds; }
		void SetInterval(uint32_t milliseconds) { m_interval = std::chrono::milliseconds(milliseconds); }
		Event<ArgTimer>& GetTickEvent() { return m_tick; }
		bool IsRunning() const { return m_isRunning.load(); }
	private:
		void Run();

		std::atomic_bool m_isRunning{ false };
		std::chrono::milliseconds m_interval{ 1000 };
		
		std::thread m_timerThread;
		Event<ArgTimer> m_tick;
	};
}

#endif