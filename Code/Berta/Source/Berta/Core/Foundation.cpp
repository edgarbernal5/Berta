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
		return g_foundation;
	}

	Foundation::RootGuard::RootGuard(Window* window) : m_window(window)
	{
	}

	Foundation::RootGuard::~RootGuard()
	{
	}

	void Foundation::EventEnterSizeMove(Window* window)
	{
		ArgSizeMove argSizeMove;
		auto events = dynamic_cast<FormEvents*>(window->Events.get());
		events->EnterSizeMove.Emit(argSizeMove);

		for (auto& child : window->Children)
		{
			if (!child->Visible) continue;

			m_windowManager.EnterSizeMove(child);
		}
	}

	void Foundation::EventExitSizeMove(Window* window)
	{
		ArgSizeMove argSizeMove;
		auto events = dynamic_cast<FormEvents*>(window->Events.get());
		events->ExitSizeMove.Emit(argSizeMove);

		for (auto& child : window->Children)
		{
			if (!child->Visible) continue;

			m_windowManager.ExitSizeMove(child);
		}
	}
}