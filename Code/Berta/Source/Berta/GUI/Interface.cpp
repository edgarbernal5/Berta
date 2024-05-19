/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#include "btpch.h"
#include "Interface.h"

#include "Berta/API/WindowAPI.h"
#include "Berta/Core/Foundation.h"
#include "Berta/GUI/Control.h"
#include "Berta/GUI/ControlAppearance.h"
#include "Berta/GUI/Caret.h"

namespace Berta::GUI
{
	Window* CreateForm(const Rectangle& rectangle, const FormStyle& formStyle)
	{
		auto windowResult = API::CreateNativeWindow(rectangle, formStyle);

		if (windowResult.WindowHandle.Handle)
		{
			auto& windowManager = Foundation::GetInstance().GetWindowManager();
			Window* window = new Window(WindowType::Native);
			window->RootHandle = windowResult.WindowHandle;
			window->Size = windowResult.ClientSize;
			window->RootWindow = window;
			window->DPI = windowResult.DPI;

			windowManager.AddNative(windowResult.WindowHandle, WindowManager::RootData(window, window->Size));
			windowManager.Add(window);

			auto& rootGraphics = windowManager.GetWindowData(windowResult.WindowHandle)->RootGraphics;
			window->RootGraphics = &rootGraphics;

			return window;
		}

		return nullptr;
	}

	Window* CreateControl(Window* parent, const Rectangle& rectangle)
	{
		auto& windowManager = Foundation::GetInstance().GetWindowManager();
		Window* window = new Window(WindowType::Control);
		window->Size = rectangle;
		window->Parent = parent;
		window->Position = rectangle;

		if (parent)
		{
			window->DPI = parent->DPI;
			window->RootWindow = parent->RootWindow;
			window->RootGraphics = parent->RootGraphics;

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
			{
				return API::GetCaptionNativeWindow(window->RootHandle);
			}

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

	void InitRenderer(ControlBase* control, ControlRenderer& controlRenderer)
	{
		auto window = control->Handle();
		auto& windowManager = Foundation::GetInstance().GetWindowManager();
		if (windowManager.Exists(window))
		{
			auto& graphics = window->Renderer.GetGraphics();
			graphics.Build(window->Size);
			graphics.BuildFont(window->DPI);
			graphics.DrawRectangle(window->Size.ToRectangle(), window->Appereance->Background, true);
			window->Renderer.Init(*control, controlRenderer);
			window->Renderer.Update();
		}
	}

	void SetEvents(Window* window, std::shared_ptr<CommonEvents> events)
	{
		auto& windowManager = Foundation::GetInstance().GetWindowManager();
		if (windowManager.Exists(window))
		{
			window->Events = events;
		}
	}

	void SetAppearance(Window* window, ControlAppearance* controlAppearance)
	{
		auto& windowManager = Foundation::GetInstance().GetWindowManager();
		if (windowManager.Exists(window))
		{
			window->Appereance = controlAppearance;
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

	Color GetBoxBackgroundColor(Window* window)
	{
		auto& windowManager = Foundation::GetInstance().GetWindowManager();
		if (windowManager.Exists(window))
		{
			return window->Appereance->BoxBackground;
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

	void UpdateDeferred(Window* window)
	{
		auto& windowManager = Foundation::GetInstance().GetWindowManager();
		if (windowManager.Exists(window))
		{
			window->RootWindow->DeferredRequests.push_back(window);
		}
	}

	void CreateCaret(Window* window)
	{
		auto& windowManager = Foundation::GetInstance().GetWindowManager();
		if (windowManager.Exists(window))
		{
			window->Caret = new Caret();
		}
	}
}