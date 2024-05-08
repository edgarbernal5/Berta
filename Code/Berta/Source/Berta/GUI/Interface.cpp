/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#include "btpch.h"
#include "Interface.h"

#include "Berta/API/WindowAPI.h"
#include "Berta/Core/Foundation.h"
#include "Berta/GUI/Widget.h"
#include "Berta/GUI/WidgetAppearance.h"

namespace Berta::GUI
{
	Window* CreateForm(const Rectangle& rectangle, const FormStyle& formStyle)
	{
		auto windowResult = API::CreateNativeWindow(rectangle, formStyle);

		if (windowResult.WindowHandle.Handle)
		{
			auto& windowManager = Foundation::GetInstance().GetWindowManager();
			Window* window = new Window(WindowType::Native);
			window->Root = windowResult.WindowHandle;
			window->Size = windowResult.ClientSize;

			windowManager.AddNative(windowResult.WindowHandle, WindowManager::WindowData(window, window->Size));
			windowManager.Add(window);

			auto& rootGraphics = windowManager.GetWindowData(windowResult.WindowHandle)->RootGraphics;
			window->RootGraphics = &rootGraphics;

			return window;
		}

		return nullptr;
	}

	Window* CreateWidget(Window* parent, const Rectangle& rectangle)
	{
		auto& windowManager = Foundation::GetInstance().GetWindowManager();
		Window* window = new Window(WindowType::Widget);
		window->Size = rectangle;
		window->Parent = parent;

		if (parent)
		{
			parent->Children.emplace_back(window);
		}

		windowManager.Add(window);
		return window;
	}

	void CaptionWindow(Window* window, const std::wstring& caption)
	{
		auto& windowManager = Foundation::GetInstance().GetWindowManager();
		if (windowManager.Exists(window))
		{
			windowManager.Caption(window, caption);
		}
	}

	std::wstring GetCaptionWindow(Window* window)
	{
		auto& windowManager = Foundation::GetInstance().GetWindowManager();
		if (windowManager.Exists(window))
		{
			if (window->Type == WindowType::Native)
				return API::GetCaptionNativeWindow(window->Root);

			return window->Title;
		}
		return {};
	}

	void DisposeWindow(Window* window)
	{
		auto& windowManager = Foundation::GetInstance().GetWindowManager();
		if (windowManager.Exists(window))
		{
			windowManager.Dispose(window);
		}
	}

	void ShowWindow(Window* window, bool visible)
	{
		auto& windowManager = Foundation::GetInstance().GetWindowManager();
		if (windowManager.Exists(window))
		{
			windowManager.Show(window, visible);
		}
	}

	void InitRenderer(WidgetBase* widget, WidgetRenderer& widgetRenderer)
	{
		auto window = widget->Handle();
		auto& windowManager = Foundation::GetInstance().GetWindowManager();
		if (windowManager.Exists(window))
		{
			auto& graphics = window->Renderer.GetGraphics();
			graphics.Build(window->Size);
			graphics.DrawRectangle(window->Size.ToRectangle(), window->Appereance->Background, true);
			window->Renderer.Init(*widget, widgetRenderer);
			window->Renderer.Update();
		}
	}

	void SetAppearance(Window* window, WidgetAppearance* widgetAppearance)
	{
		auto& windowManager = Foundation::GetInstance().GetWindowManager();
		if (windowManager.Exists(window))
		{
			window->Appereance = widgetAppearance;
		}
	}

	Color GetBackgroundColor(Window* window)
	{
		auto& windowManager = Foundation::GetInstance().GetWindowManager();
		if (windowManager.Exists(window))
		{
			return window->Appereance->Background;
		}
		return {};
	}

	Color GetForegroundColor(Window* window)
	{
		auto& windowManager = Foundation::GetInstance().GetWindowManager();
		if (windowManager.Exists(window))
		{
			return window->Appereance->Foreground;
		}
		return {};
	}
}