/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#ifndef BT_WINDOW_MANAGER_HEADER
#define BT_WINDOW_MANAGER_HEADER

#include <map>
#include <set>
#include <string>
#include <iostream>
#include "Berta/API/WindowAPI.h"
#include "Berta/Paint/Graphics.h"

namespace Berta
{
	struct Window;
	class MenuBarItemReactor;
	class MenuItemReactor;

	class WindowManager
	{
	public:
		struct RootData
		{
			Window* WindowPtr{ nullptr };
			Graphics RootGraphics;

			Window* Pressed{ nullptr };
			Window* Hovered{ nullptr };
			Window* Focused{ nullptr };
			Window* Released{ nullptr }; // Handle double-click

			API::NativeCursor CurrentCursor;

#ifdef BT_PLATFORM_WINDOWS
			TRACKMOUSEEVENT TrackEvent = { sizeof(TRACKMOUSEEVENT), TME_LEAVE };
			bool IsTracking{ false };
#endif

			RootData(RootData&& other) noexcept;
			RootData(Window* window, const Size& size);
		private:
			RootData(const RootData&) = delete;
			RootData& operator=(const RootData&) = delete;
		};

		void Add(Window* window);
		void AddNative(API::NativeWindowHandle nativeWindowHandle, RootData&& append);
		void Caption(Window* window, const std::wstring& caption);
		void Destroy(Window* window);
		void Dispose(Window* window);
		void Remove(Window* window);
		Window* Get(API::NativeWindowHandle nativeWindowHandle) const;
		RootData* GetWindowData(API::NativeWindowHandle nativeWindowHandle);
		bool Exists(Window* window) const;
		uint32_t NativeWindowCount();

		void Capture(Window* window);
		void ReleaseCapture(Window* window);

		Window* Find(Window* window, const Point& point);
		void UpdateTree(Window* window);
		void Show(Window* window, bool visible);

		void Resize(Window* window, const Size& newSize);
		void UpdateDeferredRequests(Window* rootWindow);

		void ChangeDPI(Window* window, uint32_t newDPI);
		void ChangeCursor(Window* window, Cursor newCursor);

		Point GetAbsolutePosition(Window* window);

		void SetMenu(MenuItemReactor* rootMenuItemWindow, MenuBarItemReactor* menuBarItemReactor);
		std::pair<MenuBarItemReactor*, MenuItemReactor*> GetMenu();

		void DisposeMenu(bool disposeRoot);
		void DisposeMenu(MenuItemReactor* rootReactor);
	private:
		bool IsPointOnWindow(Window* window, const Point& point);
		Window* FindInTree(Window* window, const Point& point);
		void DestroyInternal(Window* window);
		void UpdateTreeInternal(Window* window, Graphics& rootGraphics);
		void UpdateDeferredRequestsInternal(Window* request, Graphics& rootGraphics);

		struct CaptureData
		{
			Window* WindowPtr{ nullptr };
			std::vector<Window*> PrevCaptured;
		}m_capture;

		std::map<API::NativeWindowHandle, RootData> m_windowNativeRegistry;
		std::set<Window*> m_windowRegistry;

		MenuItemReactor* m_rootMenuItemReactor{ nullptr };
		MenuBarItemReactor* m_menuBarItemReactor{ nullptr };
		bool m_keyboardCaptured{ false };
	};
}

#endif