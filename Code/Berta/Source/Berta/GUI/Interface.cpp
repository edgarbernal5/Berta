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
	BasicWindow* CreateBasicWindow(const Rectangle& rectangle)
	{
		auto nativeHandle = API::CreateNativeWindow(rectangle);

		if (nativeHandle.Handle)
		{
			auto& windowManager = Foundation::GetInstance().GetWindowManager();
			BasicWindow* basicWindow = new BasicWindow();
			basicWindow->Root = nativeHandle;

			windowManager.Add(basicWindow);
			return basicWindow;
		}
		return nullptr;
	}

	void ShowBasicWindow(BasicWindow* basicWindow, bool visible)
	{
		auto& windowManager = Foundation::GetInstance().GetWindowManager();
		windowManager.Show(basicWindow, visible);
	}
}