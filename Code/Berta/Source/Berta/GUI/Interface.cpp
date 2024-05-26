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
	Window* CreateForm(Window* parent, const Rectangle& rectangle, const FormStyle& formStyle)
	{
		API::NativeWindowHandle parentHandle{};
		if (parent)
		{
			parentHandle = parent->RootWindow->RootHandle;
		}
		auto windowResult = API::CreateNativeWindow(parentHandle, rectangle, formStyle);
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
		
		Rectangle rect{ rectangle };
		if (parent && parent->DPI != 96)
		{
			float scalingFactor = parent->DPI / 96.0f;
			rect.X = rect.X * scalingFactor;
			rect.Y = rect.Y * scalingFactor;
			rect.Width = rect.Width * scalingFactor;
			rect.Height = rect.Height * scalingFactor;
		}
		window->Size = rect;
		window->Parent = parent;
		window->Position = rect;

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

	void RefreshWindow(Window* window)
	{
		auto& windowManager = Foundation::GetInstance().GetWindowManager();
		if (windowManager.Exists(window))
		{
			auto& rootGraphics = *(window->RootWindow->RootGraphics);
			Rectangle requestRectangle{ window->Position.X, window->Position.Y, window->Size.Width, window->Size.Height };

			rootGraphics.BitBlt(requestRectangle, window->Renderer.GetGraphics(), { 0,0 }); // Copy from control's graphics to root graphics.

			window->RootWindow->Renderer.Map(window->RootWindow, requestRectangle); // Copy from root graphics to native hwnd window.
		}
	}

	void MakeWindowActive(Window* window, bool active)
	{
		auto& windowManager = Foundation::GetInstance().GetWindowManager();
		if (windowManager.Exists(window))
		{
			window->MakeActive = active;
		}
	}

	void InitRendererReactor(ControlBase* control, ControlReactor& controlReactor)
	{
		auto window = control->Handle();
		auto& windowManager = Foundation::GetInstance().GetWindowManager();
		if (windowManager.Exists(window))
		{
			auto& graphics = window->Renderer.GetGraphics();
			graphics.Build(window->Size);
			graphics.BuildFont(window->DPI);
			graphics.DrawRectangle(window->Size.ToRectangle(), window->Appereance->Background, true);
			window->Renderer.Init(*control, controlReactor);
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

	void ChangeCursor(Window* window, Cursor newCursor)
	{
		auto& windowManager = Foundation::GetInstance().GetWindowManager();
		if (windowManager.Exists(window))
		{
			windowManager.ChangeCursor(window, newCursor);
		}
	}
	Rectangle GetCenteredOnScreen(uint32_t width, uint32_t height)
	{
		auto primaryScreen = API::GetPrimaryMonitorSize();
		return Rectangle{
			static_cast<int>((primaryScreen.Width - width) >> 1),
			static_cast<int>((primaryScreen.Height - height) >> 1),
			width,
			height
		};
	}

	Rectangle GetCenteredOnScreen(const Size& size)
	{
		auto primaryScreen = API::GetPrimaryMonitorSize();
		return Rectangle{
			static_cast<int>((primaryScreen.Width - size.Width) >> 1),
			static_cast<int>((primaryScreen.Height - size.Height) >> 1),
			size.Width,
			size.Height
		};
	}
	Point GetPointClientToScreen(Window* window, const Point& point)
	{
		auto& windowManager = Foundation::GetInstance().GetWindowManager();
		if (windowManager.Exists(window))
		{
			return API::GetPointClientToScreen(window->RootWindow->RootHandle, point);
		}
		return {};
	}
}