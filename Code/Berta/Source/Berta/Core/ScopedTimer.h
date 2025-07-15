/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#ifndef BT_SCOPED_TIMER_HEADER
#define BT_SCOPED_TIMER_HEADER

#include <chrono>
#include <string>
#include <iostream>

namespace Berta
{
	class ScopedTimer
	{
	public:
		ScopedTimer(const std::string& label);
		~ScopedTimer();

	private:
		std::string m_label;
		std::chrono::high_resolution_clock::time_point m_start;
	};
}

#endif