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

	void WindowManager::AddNative(API::NativeWindowHandle nativeWindowHandle, BasicWindow* basicWindow)
	{
		m_windowNativeRegistry.insert({ nativeWindowHandle, basicWindow });
	}

	void WindowManager::Caption(BasicWindow* basicWindow, const std::wstring& caption)
	{
		basicWindow->Title = caption;
		if (basicWindow->Type == WindowType::Native)
			API::CaptionNativeWindow(basicWindow->Root, caption);
	}

	void WindowManager::Destroy(BasicWindow* basicWindow)
	{
		if (basicWindow->Type == WindowType::Native)
		{
			API::DestroyNativeWindow(basicWindow->Root);
		}
	}

	BasicWindow* WindowManager::Get(API::NativeWindowHandle nativeWindowHandle)
	{
		auto it = m_windowNativeRegistry.find(nativeWindowHandle);
		if (it != m_windowNativeRegistry.end())
			return it->second;

		return nullptr;
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