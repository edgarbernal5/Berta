/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#include "btpch.h"
#include "Interface.h"

#include "Berta/Core/Foundation.h"
#include "Berta/API/WindowAPI.h"

namespace Berta::GUI
{
	BasicWindow* CreateBasicWindow(const Rectangle& rectangle, const WindowStyle& windowStyle)
	{
		auto nativeHandle = API::CreateNativeWindow(rectangle, windowStyle);

		if (nativeHandle.Handle)
		{
			auto& windowManager = Foundation::GetInstance().GetWindowManager();
			BasicWindow* basicWindow = new BasicWindow(WindowType::Native);
			basicWindow->Root = nativeHandle;

			windowManager.AddNative(nativeHandle, basicWindow);
			windowManager.Add(basicWindow);
			return basicWindow;
		}
		return nullptr;
	}

	BasicWindow* CreateWidget(const Rectangle& rectangle)
	{
		auto& windowManager = Foundation::GetInstance().GetWindowManager();
		BasicWindow* basicWindow = new BasicWindow(WindowType::Widget);

		windowManager.Add(basicWindow);
		return basicWindow;
	}

	void CaptionWindow(BasicWindow* basicWindow, const std::wstring& caption)
	{
		auto& windowManager = Foundation::GetInstance().GetWindowManager();
		if (windowManager.Exists(basicWindow))
		{
			windowManager.Caption(basicWindow, caption);
		}
	}

	void DestroyWindow(BasicWindow* basicWindow)
	{
		auto& windowManager = Foundation::GetInstance().GetWindowManager();
		if (windowManager.Exists(basicWindow))
		{
			windowManager.Destroy(basicWindow);
		}
	}

	void ShowBasicWindow(BasicWindow* basicWindow, bool visible)
	{
		auto& windowManager = Foundation::GetInstance().GetWindowManager();
		if (windowManager.Exists(basicWindow))
		{
			windowManager.Show(basicWindow, visible);
		}
	}
}