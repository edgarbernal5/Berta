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
#include "Berta/Controls/MenuBar.h"

namespace Berta::GUI
{
	Window* CreateForm(Window* parent, const Rectangle& rectangle, const FormStyle& formStyle, ControlBase* control)
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
			Window* window = new Window(WindowType::Form);
			window->RootHandle = windowResult.WindowHandle;
			window->Size = windowResult.ClientSize;
			window->RootWindow = window;
			window->DPI = windowResult.DPI;
			window->DPIScaleFactor = windowResult.DPI / 96.0f;
			window->ControlWindowPtr = std::make_unique<ControlBase::ControlWindow>(*control);

			windowManager.AddNative(windowResult.WindowHandle, WindowManager::RootData(window, window->Size));
			windowManager.Add(window);

			auto& rootGraphics = windowManager.GetWindowData(windowResult.WindowHandle)->RootGraphics;
			window->RootGraphics = &rootGraphics;

			window->Owner = parent;
			window->Parent = nullptr;

			return window;
		}

		return nullptr;
	}

	Window* CreateControl(Window* parent, bool isUnscaleRect, const Rectangle& rectangle, ControlBase* control, bool isPanel)
	{
		auto& windowManager = Foundation::GetInstance().GetWindowManager();
		Window* window = new Window(isPanel ? WindowType::Panel : WindowType::Control);
		window->ControlWindowPtr = std::make_unique<ControlBase::ControlWindow>(*control);
		
		Rectangle rect{ rectangle };
		if (isUnscaleRect && parent && parent->DPI != 96u)
		{
			float scalingFactor = parent->DPI / 96.0f;
			rect.X = static_cast<int>(rect.X * scalingFactor);
			rect.Y = static_cast<int>(rect.Y * scalingFactor);
			rect.Width = static_cast<uint32_t>(rect.Width * scalingFactor);
			rect.Height = static_cast<uint32_t>(rect.Height * scalingFactor);
		}
		window->Size = rect;
		window->Parent = parent;
		window->Position = rect;

		if (parent)
		{
			window->DPI = parent->DPI;
			window->DPIScaleFactor = parent->DPIScaleFactor;
			window->RootWindow = parent->RootWindow;
			window->RootHandle = parent->RootHandle;
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

	std::wstring CaptionWindow(Window* window)
	{
		auto& windowManager = Foundation::GetInstance().GetWindowManager();
		if (windowManager.Exists(window))
		{
			if (window->Type == WindowType::Form)
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
		if (windowManager.Exists(window) && !window->Flags.IsDestroyed)
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
			windowManager.Refresh(window);
		}
	}

	void EnableWindow(Window* window, bool isEnabled)
	{
		auto& windowManager = Foundation::GetInstance().GetWindowManager();
		if (windowManager.Exists(window) && window->Flags.IsEnabled != isEnabled)
		{
			window->Flags.IsEnabled = isEnabled;
			window->Renderer.Update();
			RefreshWindow(window);
			if (window->Type == WindowType::Form)
			{
				API::EnableWindow(window->RootHandle, isEnabled);
			}
		}
	}

	bool EnableWindow(Window* window)
	{
		auto& windowManager = Foundation::GetInstance().GetWindowManager();
		if (windowManager.Exists(window))
		{
			return window->Flags.IsEnabled;
			
		}
		return false;
	}

	void ResizeWindow(Window* window, const Size& newSize)
	{
		auto& windowManager = Foundation::GetInstance().GetWindowManager();
		if (windowManager.Exists(window))
		{
			windowManager.Resize(window, newSize);
		}
	}

	Size SizeWindow(Window* window)
	{
		auto& windowManager = Foundation::GetInstance().GetWindowManager();
		if (windowManager.Exists(window))
		{
			return window->Size;
		}
		return {};
	}

	void MoveWindow(Window* window, const Rectangle& newRect)
	{
		auto& windowManager = Foundation::GetInstance().GetWindowManager();
		if (windowManager.Exists(window))
		{
			windowManager.Move(window, newRect);
		}
	}

	void MakeWindowActive(Window* window, bool active)
	{
		auto& windowManager = Foundation::GetInstance().GetWindowManager();
		if (windowManager.Exists(window))
		{
			window->Flags.MakeActive = active;
		}
	}

	void Capture(Window* window)
	{
		auto& windowManager = Foundation::GetInstance().GetWindowManager();
		if (windowManager.Exists(window))
		{
			windowManager.Capture(window);
		}
	}

	void ReleaseCapture(Window* window)
	{
		auto& windowManager = Foundation::GetInstance().GetWindowManager();
		if (windowManager.Exists(window))
		{
			windowManager.ReleaseCapture(window);
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

	void SetEvents(Window* window, std::shared_ptr<ControlEvents> events)
	{
		auto& windowManager = Foundation::GetInstance().GetWindowManager();
		if (windowManager.Exists(window))
		{
			window->Events = events;
		}
	}

	void SetAppearance(Window* window, std::shared_ptr<ControlAppearance> controlAppearance)
	{
		auto& windowManager = Foundation::GetInstance().GetWindowManager();
		if (windowManager.Exists(window))
		{
			window->Appereance = controlAppearance;
		}
	}

	Point GetAbsolutePosition(Window* window)
	{
		auto& windowManager = Foundation::GetInstance().GetWindowManager();
		if (windowManager.Exists(window))
		{
			return windowManager.GetAbsolutePosition(window);
		}
		return {};
	}

	Point GetMousePositionToWindow(Window* window)
	{
		auto& windowManager = Foundation::GetInstance().GetWindowManager();
		if (windowManager.Exists(window))
		{
			auto mousePosition = API::GetPointScreenToClient(window->RootHandle, API::GetScreenMousePosition());

			return mousePosition - windowManager.GetAbsolutePosition(window);
		}
		return {};
	}

	void UpdateTree(Window* window)
	{
		auto& windowManager = Foundation::GetInstance().GetWindowManager();
		if (windowManager.Exists(window))
		{
			windowManager.UpdateTree(window);
		}
	}

	void UpdateDeferred(Window* window)
	{
		auto& windowManager = Foundation::GetInstance().GetWindowManager();
		if (windowManager.Exists(window))
		{
			if (std::find(
				window->RootWindow->DeferredRequests.begin(),
				window->RootWindow->DeferredRequests.end(),
				window) == window->RootWindow->DeferredRequests.end()
			)
			{
				window->RootWindow->DeferredRequests.push_back(window);
			}
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
		uint32_t dpi = API::GetNativeWindowDPI({});
		float downwardScale = 96.0f / static_cast<float>(dpi);
		float upwardScale = static_cast<float>(dpi) / 96.0f;
		auto primaryScreen = API::GetPrimaryMonitorSize() * downwardScale;

		int x = static_cast<int>((primaryScreen.Width - width) >> 1);
		int y = static_cast<int>((primaryScreen.Height - height) >> 1);
		return Rectangle{
			static_cast<int>(x * upwardScale),
			static_cast<int>(y * upwardScale),
			static_cast<uint32_t>(width * upwardScale),
			static_cast<uint32_t>(height * upwardScale)
		};
	}

	Rectangle GetCenteredOnScreen(const Size& size)
	{
		uint32_t dpi = API::GetNativeWindowDPI({});
		float downwardScale = 96.0f / static_cast<float>(dpi);
		float upwardScale = static_cast<float>(dpi) / 96.0f;
		auto primaryScreen = API::GetPrimaryMonitorSize() * downwardScale;

		int x = static_cast<int>((primaryScreen.Width - size.Width) >> 1);
		int y = static_cast<int>((primaryScreen.Height - size.Height) >> 1);
		return Rectangle{
			static_cast<int>(x * upwardScale),
			static_cast<int>(y * upwardScale),
			static_cast<uint32_t>(size.Width * upwardScale),
			static_cast<uint32_t>(size.Height * upwardScale)
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

	Point GetPointScreenToClient(Window* window, const Point& point)
	{
		auto& windowManager = Foundation::GetInstance().GetWindowManager();
		if (windowManager.Exists(window))
		{
			return API::GetPointScreenToClient(window->RootWindow->RootHandle, point);
		}
		return {};
	}

	void SendCustomMessage(Window* window, std::function<void()> body)
	{
		auto& windowManager = Foundation::GetInstance().GetWindowManager();
		if (windowManager.Exists(window))
		{
			API::SendCustomMessage(window->RootWindow->RootHandle, body);
		}
	}

	void SetMenu(MenuItemReactor* rootMenuItemWindow, MenuBarItemReactor* menuBarItemReactor)
	{
		auto& windowManager = Foundation::GetInstance().GetWindowManager();
		if (windowManager.Exists(rootMenuItemWindow->Owner()))
		{
			windowManager.SetMenu(rootMenuItemWindow, menuBarItemReactor);
		}
	}

	void DisposeMenu(bool disposeRoot)
	{
		auto& windowManager = Foundation::GetInstance().GetWindowManager();
		windowManager.DisposeMenu(disposeRoot);
	}

	void DisposeMenu(MenuItemReactor* rootReactor)
	{
		auto& windowManager = Foundation::GetInstance().GetWindowManager();
		windowManager.DisposeMenu(rootReactor);
	}
}