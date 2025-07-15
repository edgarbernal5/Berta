/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#include "btpch.h"
#include "ScopedTimer.h"

namespace Berta
{
	ScopedTimer::ScopedTimer(const std::string& label) : 
		m_label(label),
		m_start(std::chrono::high_resolution_clock::now())
	{
	}

	ScopedTimer::~ScopedTimer()
	{
		auto end = std::chrono::high_resolution_clock::now();
		auto duration_micro = std::chrono::duration_cast<std::chrono::microseconds>(end - m_start).count();
		std::cout << "[ScopedTimer] " << m_label << " took " << duration_micro << " microseconds.\n";
	}
}