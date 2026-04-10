/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#include "btpch.h"
#include "Interface.h"

#include <utility>

#include "Berta/API/WindowAPI.h"
#include "Berta/Core/Foundation.h"
#include "Berta/GUI/Control.h"
#include "Berta/GUI/ControlAppearance.h"
#include "Berta/GUI/Caret.h"
#include "Berta/Controls/MenuBar.h"

namespace Berta::GUI
{
	Window* CreateForm(Window* parent, bool isUnscaleRect, const Rectangle& rectangle, const FormStyle& formStyle, bool isNested, ControlBase* control, bool isRenderForm)
	{
		return Foundation::GetInstance().GetWindowManager().CreateForm(parent, isUnscaleRect, rectangle, formStyle, isNested, control, isRenderForm);
	}

	Window* CreateControl(Window* parent, bool isUnscaleRect, const Rectangle& rectangle, ControlBase* control, bool isPanel)
	{
		return Foundation::GetInstance().GetWindowManager().CreateControl(parent, isUnscaleRect, rectangle, control, isPanel);
	}

	API::NativeWindowHandle GetNativeHandle(Window* window)
	{
		auto& windowManager = Foundation::GetInstance().GetWindowManager();
		if (!windowManager.Exists(window))
		{
			return {};
		}

		return window->RootHandle;
	}

	void CaptionWindow(Window* window, const std::wstring& caption)
	{
		auto& windowManager = Foundation::GetInstance().GetWindowManager();
		if (!windowManager.Exists(window))
		{
			return;
		}

		if (windowManager.Caption(window, caption))
		{
			windowManager.Update(window, true);
		}
	}

	std::wstring CaptionWindow(Window* window)
	{
		auto& windowManager = Foundation::GetInstance().GetWindowManager();
		if (!windowManager.Exists(window))
		{
			return {};
		}

		if (window->IsNative())
		{
			window->Title = API::GetCaptionNativeWindow(window->RootHandle);
		}

		return window->Title;
	}

	void DisposeWindow(Window* window)
	{
		auto& windowManager = Foundation::GetInstance().GetWindowManager();
		if (!windowManager.Exists(window) || window->Flags.IsDisposed)
		{
			return;
		}

		windowManager.Dispose(window);
	}

	void ShowWindow(Window* window, bool visible)
	{
		auto& windowManager = Foundation::GetInstance().GetWindowManager();
		if (!windowManager.Exists(window))
		{
			return;
		}

		windowManager.Show(window, visible);
	}

	bool IsWindowVisible(Window* window)
	{
		auto& windowManager = Foundation::GetInstance().GetWindowManager();
		if (!windowManager.Exists(window))
		{
			return false;
		}

		return window->IsVisible();
	}

	void UpdateWindow(Window* window)
	{
		auto& windowManager = Foundation::GetInstance().GetWindowManager();
		if (!windowManager.Exists(window))
		{
			return;
		}

		windowManager.Update(window, true);
	}

	void EnableWindow(Window* window, bool isEnabled)
	{
		auto& windowManager = Foundation::GetInstance().GetWindowManager();
		if (!windowManager.Exists(window) || window->Flags.IsEnabled == isEnabled)
		{
			return;
		}

		window->Flags.IsEnabled = isEnabled;
		UpdateWindow(window);

		if (window->IsNative())
		{
			API::EnableWindow(window->RootHandle, isEnabled);
		}
	}

	bool IsWindowEnabled(Window* window)
	{
		auto& windowManager = Foundation::GetInstance().GetWindowManager();
		if (!windowManager.Exists(window))
		{
			return false;
		}

		return window->Flags.IsEnabled;
	}

	void RefreshWindow(Window* window)
	{
		auto& windowManager = Foundation::GetInstance().GetWindowManager();
		if (!windowManager.Exists(window))
		{
			return;
		}

		windowManager.Refresh(window);
	}

	void ResizeWindow(Window* window, const Size& newSize)
	{
		auto& windowManager = Foundation::GetInstance().GetWindowManager();
		if (!windowManager.Exists(window))
		{
			return;
		}

		if (windowManager.Resize(window, newSize))
		{
			auto windowToUpdate = window;
			if (!window->IsNative())
			{
				windowToUpdate = windowToUpdate->FindFirstNonPanelAncestor();
			}

			windowManager.Update(windowToUpdate, false);
		}
	}

	Size SizeWindow(Window* window)
	{
		auto& windowManager = Foundation::GetInstance().GetWindowManager();
		if (!windowManager.Exists(window))
		{
			return {};
		}

		return window->ClientSize;
	}

	bool MoveWindow(Window* window, const Rectangle& newRect, bool forceRepaint)
	{
		auto& windowManager = Foundation::GetInstance().GetWindowManager();
		if (!windowManager.Exists(window))
		{
			return false;
		}

		bool hasChanged = windowManager.Move(window, newRect, forceRepaint);
		if (hasChanged)
		{
			auto windowToUpdate = window;
			if (!window->IsNative())
			{
				windowToUpdate = windowToUpdate->FindFirstNonPanelAncestor();
			}

			windowManager.Update(windowToUpdate, false);
		}

		return hasChanged;
	}

	bool MoveWindow(Window* window, const Point& newPosition, bool forceRepaint)
	{
		auto& windowManager = Foundation::GetInstance().GetWindowManager();
		if (!windowManager.Exists(window))
		{
			return false;
		}

		bool hasChanged = windowManager.Move(window, newPosition, forceRepaint);
		if (hasChanged)
		{
			auto windowToUpdate = window;
			if (!window->IsNative())
			{
				windowToUpdate = windowToUpdate->FindFirstNonPanelAncestor();
			}

			if (windowToUpdate != nullptr)
			{
				windowManager.Update(window, false);
			}
		}

		return hasChanged;
	}

	Rectangle AreaWindow(Window* window)
	{
		auto& windowManager = Foundation::GetInstance().GetWindowManager();
		if (!windowManager.Exists(window))
		{
			return{};
		}

		auto position = GetWindowRootPosition(window);
		return { position.X, position.Y, window->ClientSize.Width, window->ClientSize.Height };
	}

	void MakeWindowActive(Window* window, bool active, Window* makeTargetWhenInactive)
	{
		auto& windowManager = Foundation::GetInstance().GetWindowManager();
		if (!windowManager.Exists(window))
		{
			return;
		}

		if (active)
		{
			makeTargetWhenInactive = nullptr;
		}
		window->Flags.MakeActive = active;
		window->MakeTargetWhenInactive = makeTargetWhenInactive;
	}

	void FocusWindow(Window* window)
	{
		auto& windowManager = Foundation::GetInstance().GetWindowManager();
		if (!windowManager.Exists(window))
		{
			return;
		}
		
		windowManager.Focus(window, ArgFocus::Reason::General);
	}

	Window* GetParentWindow(Window* window)
	{
		auto& windowManager = Foundation::GetInstance().GetWindowManager();
		if (!windowManager.Exists(window))
		{
			return nullptr;
		}

		if (window->IsNative())
		{
			auto rootWindow = windowManager.Get(API::GetParentWindow(window->RootHandle));
			return rootWindow;
		}

		return window->Parent;
	}

	Window* GetOwnerWindow(Window* window)
	{
		auto& windowManager = Foundation::GetInstance().GetWindowManager();
		if (!windowManager.Exists(window))
		{
			return nullptr;
		}

		if (window->Owner)
		{
			return window->Owner;
		}

		if (window->IsNative())
		{
			auto rootWindow = windowManager.Get(API::GetOwnerWindow(window->RootHandle));
			return rootWindow;
		}

		return nullptr;
	}

	void Capture(Window* window, bool redirectToChildren)
	{
		auto& windowManager = Foundation::GetInstance().GetWindowManager();
		if (!windowManager.Exists(window))
		{
			return;
		}

		windowManager.Capture(window, redirectToChildren);
	}

	void ReleaseCapture(Window* window)
	{
		auto& windowManager = Foundation::GetInstance().GetWindowManager();
		if (!windowManager.Exists(window))
		{
			return;
		}

		windowManager.ReleaseCapture(window);
	}

	void InitRendererReactor(ControlBase* control, ControlReactor& reactor)
	{
		auto window = control->Handle();
		auto& windowManager = Foundation::GetInstance().GetWindowManager();
		if (!windowManager.Exists(window))
		{
			return;
		}

		window->Renderer.Init(*control, reactor);
	}

	void SetEvents(Window* window, std::shared_ptr<ControlEvents> events)
	{
		auto& windowManager = Foundation::GetInstance().GetWindowManager();
		if (!windowManager.Exists(window))
		{
			return;
		}

		window->Events = std::move(events);
	}

	void SetAppearance(Window* window, std::shared_ptr<ControlAppearance> appearance)
	{
		auto& windowManager = Foundation::GetInstance().GetWindowManager();
		if (!windowManager.Exists(window))
		{
			return;
		}

		window->Appearance = std::move(appearance);
	}

	void SetCustomPaintCallback(Window* window, std::function<void()> callback)
	{
		auto& windowManager = Foundation::GetInstance().GetWindowManager();
		if (!windowManager.Exists(window) || !window->IsNative())
		{
			return;
		}

		window->RenderForAttributes.CustomPaint.swap(callback);
	}

	Point GetWindowRootPosition(Window* window)
	{
		auto& windowManager = Foundation::GetInstance().GetWindowManager();
		if (!windowManager.Exists(window))
		{
			return {};
		}

		return windowManager.GetWindowRootPosition(window);
	}

	Point GetWindowPosition(Window* window)
	{
		auto& windowManager = Foundation::GetInstance().GetWindowManager();
		if (!windowManager.Exists(window))
		{
			return {};
		}

		if (window->IsNative())
		{
			auto position = API::GetWindowPosition(window->RootHandle);
			if (window->Owner)
			{
				return position - window->Owner->PositionRoot;
			}

			return position;
		}

		return windowManager.GetWindowPosition(window);
	}

	Point GetMousePositionToWindow(Window* window)
	{
		auto& windowManager = Foundation::GetInstance().GetWindowManager();
		if (!windowManager.Exists(window))
		{
			return {};
		}

		auto mousePosition = API::GetPointScreenToClient(window->RootHandle, API::GetScreenMousePosition());
		return mousePosition - windowManager.GetWindowRootPosition(window);
	}

	Point GetScreenMousePosition()
	{
		return API::GetScreenMousePosition();
	}

	void SetParentWindow(Window* window, Window* newParent)
	{
		auto& windowManager = Foundation::GetInstance().GetWindowManager();
		if (!windowManager.Exists(window) || !windowManager.Exists(newParent))
		{
			return;
		}

		windowManager.SetParent(window, newParent);
	}

	void UpdateTree(Window* window, bool now)
	{
		auto& windowManager = Foundation::GetInstance().GetWindowManager();
		if (!windowManager.Exists(window))
		{
			return;
		}

		auto windowToUpdate = window->FindFirstNonPanelAncestor();
		windowManager.UpdateTree(windowToUpdate, now);
	}

	void MarkAsNeedUpdate(Window* window)
	{
		auto& windowManager = Foundation::GetInstance().GetWindowManager();
		if (!windowManager.Exists(window))
		{
			return;
		}

		window->DrawStatus = DrawWindowStatus::NeedUpdate;
	}

	void ChangeCursor(Window* window, Cursor newCursor)
	{
		auto& windowManager = Foundation::GetInstance().GetWindowManager();
		if (!windowManager.Exists(window))
		{
			return;
		}

		windowManager.ChangeCursor(window, newCursor);
	}

	Cursor GetCursor(Window* window)
	{
		auto& windowManager = Foundation::GetInstance().GetWindowManager();
		if (!windowManager.Exists(window))
		{
			return Cursor::Default;
		}

		return windowManager.GetCursor(window);
	}

	//TODO: JustCtrl_CenterWindow
	Rectangle GetCenteredOnScreen(uint32_t width, uint32_t height)
	{
		uint32_t dpi = API::GetNativeWindowDPI({});
		float downwardScale = LayoutUtils::CalculateDownwardDPIScaleFactor(dpi);
		float upwardScale = LayoutUtils::CalculateDPIScaleFactor(dpi);
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
		float downwardScale = LayoutUtils::CalculateDownwardDPIScaleFactor(dpi);
		float upwardScale = LayoutUtils::CalculateDPIScaleFactor(dpi);
		auto primaryScreen = API::GetPrimaryMonitorSize() * downwardScale;

		int x = static_cast<int>((primaryScreen.Width - size.Width) >> 1);
		int y = static_cast<int>((primaryScreen.Height - size.Height) >> 1);

		return Rectangle
		{
			static_cast<int>(x * upwardScale),
			static_cast<int>(y * upwardScale),
			static_cast<uint32_t>(size.Width * upwardScale),
			static_cast<uint32_t>(size.Height * upwardScale)
		};
	}

	Point GetPointClientToScreen(Window* window, const Point& point)
	{
		auto& windowManager = Foundation::GetInstance().GetWindowManager();
		if (!windowManager.Exists(window))
		{
			return {};
		}

		return API::GetPointClientToScreen(window->RootWindow->RootHandle, point);
	}

	Point GetPointScreenToClient(Window* window, const Point& point)
	{
		auto& windowManager = Foundation::GetInstance().GetWindowManager();
		if (!windowManager.Exists(window))
		{
			return {};
		}

		return API::GetPointScreenToClient(window->RootWindow->RootHandle, point);
	}

	void SendCustomMessage(Window* window, std::function<void()> body)
	{
		auto& windowManager = Foundation::GetInstance().GetWindowManager();
		if (!windowManager.Exists(window))
		{
			return;
		}

		API::SendCustomMessage(window->RootWindow->RootHandle, body);
	}

	bool IsWindowBorderless(Window* window)
	{
		auto& windowManager = Foundation::GetInstance().GetWindowManager();
		if (!windowManager.Exists(window))
		{
			return false;
		}
		
		return window->Flags.Borderless;
	}

	void DisposeMenu()
	{
		auto& menuManager = Foundation::GetInstance().GetMenuManager();
		menuManager.CloseAll();
	}

	void Exit()
	{
		auto& windowManager = Foundation::GetInstance().GetWindowManager();
		std::vector<API::NativeWindowHandle> allHandles;
		windowManager.GetNativeWindows(allHandles);

		for (auto& item : allHandles)
		{
			API::DestroyNativeWindow(item);
		}
	}

	Color GetBackgroundColor(Window* window)
	{
		auto& windowManager = Foundation::GetInstance().GetWindowManager();
		if (!windowManager.Exists(window))
		{
			return {};
		}

		return window->Appearance->Background;
	}

	void SetBackgroundColor(Window* window, const Color& newColor)
	{
		auto& windowManager = Foundation::GetInstance().GetWindowManager();
		if (!windowManager.Exists(window))
		{
			return;
		}

		window->Appearance->Background = newColor;
	}

	Window* GetMenuBar(Window* window)
	{
		auto& windowManager = Foundation::GetInstance().GetWindowManager();
		if (windowManager.Exists(window))
		{
			return window->RootWindow->MenuBar;
		}

		return nullptr;
	}

	void SetMenuBar(Window* menuBar)
	{
		auto& windowManager = Foundation::GetInstance().GetWindowManager();
		if (!windowManager.Exists(menuBar))
		{
			return;
		}

		menuBar->RootWindow->MenuBar = menuBar;
	}

	std::wstring GetAccessKeyText(const std::wstring& text, wchar_t &accessKey, std::size_t* accessKeyPosition)
	{
		accessKey = 0;
		if (accessKeyPosition)
		{
			*accessKeyPosition = std::wstring::npos;
		}

		std::wstring result;
		result.reserve(text.size());

		for (std::size_t i = 0; i < text.size(); ++i)
		{
			if (text[i] == L'&')
			{
				if (i + 1 < text.size())
				{
					if (text[i + 1] == L'&')
					{
						result.push_back(L'&');
						++i;
					}
					else if (accessKey == 0)
					{
						accessKey = text[i + 1];
						if (accessKeyPosition)
						{
							*accessKeyPosition = result.size();
						}
					}
				}
			}
			else
			{
				result.push_back(text[i]);
			}
		}

		return result;
	}

	void DrawAccessKeyUnderline(Graphics& graphics, const std::wstring& wstr, wchar_t accessKey, std::size_t accessKeyPosition, const Point& position, const Color& color)
	{
		if (!accessKey)
		{
			return;
		}
		if (accessKeyPosition == std::wstring::npos || accessKeyPosition >= wstr.size())
		{
			return;
		}

		auto textSize = graphics.GetTextExtent(wstr.substr(0, accessKeyPosition + 1));
		auto accessKeyTextSize = graphics.GetTextExtent(std::wstring{ wstr[accessKeyPosition] });

		auto underlineStart = Point{ position.X + static_cast<int>(textSize.Width - accessKeyTextSize.Width), position.Y + static_cast<int>(accessKeyTextSize.Height) - 2 };
		auto underlineEnd = Point{ underlineStart.X + static_cast<int>(accessKeyTextSize.Width), underlineStart.Y };

		graphics.DrawLine(underlineStart, underlineEnd, color);
	}
}
