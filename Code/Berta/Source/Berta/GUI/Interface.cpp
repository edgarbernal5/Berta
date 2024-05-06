/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#include "btpch.h"
#include "Interface.h"

#include "Berta/API/WindowAPI.h"
#include "Berta/Core/Foundation.h"
#include "Berta/Core/Widget.h"
#include "Berta/GUI/WidgetAppearance.h"

namespace Berta::GUI
{
	BasicWindow* CreateNativeWindow(const Rectangle& rectangle, const WindowStyle& windowStyle)
	{
		auto windowResult = API::CreateNativeWindow(rectangle, windowStyle);

		if (windowResult.WindowHandle.Handle)
		{
			auto& windowManager = Foundation::GetInstance().GetWindowManager();
			BasicWindow* basicWindow = new BasicWindow(WindowType::Native);
			basicWindow->Root = windowResult.WindowHandle;
			basicWindow->Size = windowResult.ClientSize;

			windowManager.AddNative(windowResult.WindowHandle, WindowManager::WindowData(basicWindow, basicWindow->Size));
			windowManager.Add(basicWindow);

			auto& rootGraphics = windowManager.GetWindowData(windowResult.WindowHandle)->RootGraphics;
			basicWindow->RootGraphics = &rootGraphics;

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

	void InitRenderer(WidgetBase* widget, WidgetRenderer& widgetRenderer)
	{
		auto basicWindow = widget->Handle();
		auto& windowManager = Foundation::GetInstance().GetWindowManager();
		if (windowManager.Exists(basicWindow))
		{
			auto& graphics = basicWindow->Renderer.GetGraphics();
			graphics.Build(basicWindow->Size);
			graphics.DrawRectangle(basicWindow->Size.ToRectangle(), basicWindow->Appereance->Background, true);
			basicWindow->Renderer.Init(*widget, widgetRenderer);
			basicWindow->Renderer.Update();
		}
	}

	void SetAppearance(BasicWindow* basicWindow, WidgetAppearance* widgetAppearance)
	{
		auto& windowManager = Foundation::GetInstance().GetWindowManager();
		if (windowManager.Exists(basicWindow))
		{
			basicWindow->Appereance = widgetAppearance;
		}
	}

	Color GetBackgroundColor(BasicWindow* basicWindow)
	{
		auto& windowManager = Foundation::GetInstance().GetWindowManager();
		if (windowManager.Exists(basicWindow))
		{
			return basicWindow->Appereance->Background;
		}
		return {};
	}

	Color GetForegroundColor(BasicWindow* basicWindow)
	{
		auto& windowManager = Foundation::GetInstance().GetWindowManager();
		if (windowManager.Exists(basicWindow))
		{
			return basicWindow->Appereance->Foreground;
		}
		return {};
	}
}