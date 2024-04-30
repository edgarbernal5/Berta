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
		m_windowRegistry[basicWindow] = basicWindow->Root;
	}

	void WindowManager::Show(BasicWindow* basicWindow, bool visible)
	{

		if (basicWindow->Visible != visible)
		{
			API::ShowNativeWindow(basicWindow->Root, visible);
		}
	}
}