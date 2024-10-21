/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#include "btpch.h"
#include "Foundation.h"

namespace Berta
{
	Foundation Foundation::g_foundation;

	Foundation& Foundation::GetInstance()
	{
		return g_foundation;
	}

	Foundation::RootGuard::RootGuard(Window* window) : m_window(window)
	{
		BT_CORE_TRACE << "ctor RootGuard" << std::endl;
		m_window->Flags.Deferred = true;
	}

	Foundation::RootGuard::~RootGuard()
	{
		BT_CORE_TRACE << "dctor RootGuard" << std::endl;
		m_window->Flags.Deferred = false;
	}
}