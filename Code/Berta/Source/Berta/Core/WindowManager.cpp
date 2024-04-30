/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#include "btpch.h"
#include "WindowManager.h"

#include "Berta/GUI/BasicWindow.h"

namespace Berta
{
	void WindowManager::Add(BasicWindow* basicWindow)
	{
		m_windowRegistry.insert(basicWindow);
	}

	void WindowManager::Show(BasicWindow* basicWindow, bool visible)
	{
		if (m_windowRegistry.find(basicWindow) == m_windowRegistry.end())
			return;

		if (basicWindow->Visible != visible)
		{
			API::ShowNativeWindow(basicWindow->Root, visible);
		}
	}
}