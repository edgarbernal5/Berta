/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#include "btpch.h"
#include "Foundation.h"

namespace Berta
{
	Foundation& Foundation::GetInstance()
	{
		static Foundation g_foundation;
		return g_foundation;
	}

	Foundation::RootGuard::RootGuard(Window* window) : m_window(window)
	{
		++m_window->Flags.IsDeferredCount;
	}

	Foundation::RootGuard::~RootGuard()
	{
		--m_window->Flags.IsDeferredCount;
	}
}