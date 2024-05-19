/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#include "btpch.h"
#include "Timer.h"

namespace Berta
{
	Timer::Timer()
	{
		m_isRunning.store(false);
	}

	Timer::~Timer()
	{
		Stop();
	}

	void Timer::Start()
	{
		if (m_isRunning)
		{
			return;
		}

		m_isRunning = true;
		timerThread = std::thread(&Timer::Run, this);
	}

	void Timer::Stop()
	{
		m_isRunning = false;
		if (timerThread.joinable())
		{
			timerThread.join();
		}
	}

	void Timer::Run()
	{
		while (m_isRunning)
		{
			std::this_thread::sleep_for(m_interval);
			if (m_isRunning)
			{
				m_tick.Emit({});
			}
		}
	}
}