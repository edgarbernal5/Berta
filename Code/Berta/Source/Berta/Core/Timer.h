/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#ifndef BT_TIMER_HEADER
#define BT_TIMER_HEADER

#include <chrono>
#include "Berta/Core/Event.h"

namespace Berta
{
	using TimerIdentifier = UINT_PTR;

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

	private:
		TimerIdentifier m_id{};
		std::chrono::milliseconds m_interval{ 1000 };
		Event<ArgTimer> m_tick;
	};
}

#endif