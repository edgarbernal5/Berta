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

	void WindowManager::Caption(BasicWindow* basicWindow, const std::wstring& caption)
	{
		if (basicWindow->Type == WindowType::Native)
			API::CaptionNativeWindow(basicWindow->Root, caption);
	}

	bool WindowManager::Exists(BasicWindow* basicWindow)
	{
		return m_windowRegistry.find(basicWindow) != m_windowRegistry.end();
	}

	void WindowManager::Show(BasicWindow* basicWindow, bool visible)
	{
		if (basicWindow->Visible != visible)
		{
			if (basicWindow->Type == WindowType::Native)
				API::ShowNativeWindow(basicWindow->Root, visible);

			basicWindow->Visible = visible;
		}
	}
}