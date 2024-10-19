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
		{
			std::lock_guard<std::mutex> lock(m_conditionMutex);
			m_isRunning.store(false);
		}
		m_conditionVariable.notify_all();
		if (m_timerThread.joinable())
		{
			m_timerThread.join();
		}
	}

	void Timer::SetInterval(std::chrono::milliseconds milliseconds)
	{
		if (m_interval.load() == milliseconds)
			return;

		if (!m_isRunning.load())
		{
			m_interval.store(milliseconds);
			return;
		}

		Stop();
		m_interval.store(milliseconds);

		Start();
	}

	void Timer::SetInterval(uint32_t milliseconds)
	{
		if (m_interval.load() == std::chrono::milliseconds(milliseconds))
		{
			return;
		}

		if (!m_isRunning.load())
		{
			m_interval.store(std::chrono::milliseconds(milliseconds));
			return;
		}

		Stop();
		m_interval.store(std::chrono::milliseconds(milliseconds));
		
		Start();
	}

	void Timer::Run()
	{
		std::unique_lock<std::mutex> lock(m_conditionMutex);
		while (m_isRunning.load())
		{
			// Wait for a signal to wake up or continue checking periodically
			m_conditionVariable.wait_for(lock, std::chrono::milliseconds(m_interval), [this] { return !m_isRunning.load(); });
			
			if (!m_isRunning.load())
			{
				break;
			}

			API::SendCustomMessage(m_owner->RootHandle, [this]()
			{
				ArgTimer argTimer;
				m_tick.Emit(argTimer);
			});
		}
	}
}