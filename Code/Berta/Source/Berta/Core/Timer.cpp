/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#include "btpch.h"
#include "Timer.h"

#include "Berta/API/WindowAPI.h"
#include "Berta/GUI/Window.h"

namespace Berta
{
	Timer::Timer(Window* owner) : 
		m_owner(owner)
	{
	}

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
		if (m_isRunning.load())
		{
			return;
		}

		m_isRunning.store(true);
		m_timerThread = std::thread(&Timer::Run, this);
	}

	void Timer::Stop()
	{
		if (!m_isRunning.load())
		{
			return;
		}

		m_isRunning.store(false);
		m_timerThread.join();
	}

	void Timer::Run()
	{
		//https://chatgpt.com/c/781edcee-9945-4f63-baba-d0ce43746f42
		
		while (m_isRunning.load())
		{
			std::this_thread::sleep_for(m_interval.load());
			if (m_isRunning.load())
			{
				API::SendCustomMessage(m_owner->RootHandle, [this]()
				{
					ArgTimer argTimer;
					m_tick.Emit(argTimer);
				});
			}
			else
				break;
		}
	}
}