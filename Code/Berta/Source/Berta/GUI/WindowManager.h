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
	class MenuItemReactor;

	class WindowManager
	{
	public:
		struct FormData
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

			FormData(FormData&& other) noexcept;
			FormData(Window* window, const Size& size);
		private:
			FormData(const FormData&) = delete;
			FormData& operator=(const FormData&) = delete;
		};

		void Add(Window* window);
		void AddNative(API::NativeWindowHandle nativeWindowHandle, FormData&& append);
		void Caption(Window* window, const std::wstring& caption);
		void Destroy(Window* window);
		void Dispose(Window* window);
		void Remove(Window* window);
		Window* Get(API::NativeWindowHandle nativeWindowHandle) const;
		FormData* GetFormData(API::NativeWindowHandle nativeWindowHandle);
		bool Exists(Window* window) const;
		uint32_t NativeWindowCount();

		void Capture(Window* window, bool redirectToChildren);
		void ReleaseCapture(Window* window);

		Window* Find(Window* window, const Point& point);
		void UpdateTree(Window* window);
		void Map(Window* window, const Rectangle* areaToUpdate);
		void Show(Window* window, bool visible);

		void Resize(Window* window, const Size& newSize, bool resizeForm = true);
		void Move(Window* window, const Rectangle& newRect);
		void Update(Window* window);
		bool TryDeferredUpdate(Window* window);
		void UpdateDeferredRequests(Window* rootWindow);
		void Paint(Window* window, bool doUpdate);

		void ChangeDPI(Window* window, uint32_t newDPI, const API::NativeWindowHandle& nativeWindowHandle);
		void ChangeCursor(Window* window, Cursor newCursor);
		Cursor GetCursor(Window* window);

		Point GetAbsolutePosition(Window* window);
		Point GetAbsoluteRootPosition(Window* window);
		Point GetLocalPosition(Window* window);

		void SetMenu(MenuItemReactor* rootMenuItemWindow);
		MenuItemReactor* GetMenu();

		void DisposeMenu();
		void DisposeMenu(MenuItemReactor* rootReactor);

	private:
		bool IsPointOnWindow(Window* window, const Point& point);
		Window* FindInTree(Window* window, const Point& point);
		void DestroyInternal(Window* window);
		void UpdateTreeInternal(Window* window, Graphics& rootGraphics);
		void PaintInternal(Window* window, Graphics& rootGraphics, bool doUpdate);
		bool GetIntersectionClipRect(const Rectangle& parentRectangle, const Rectangle& childRectangle, Rectangle& result);

		struct CaptureHistoryData
		{
			Window* WindowPtr{ nullptr };
			bool RedirectToChildren{ false };

			CaptureHistoryData() = default;
			CaptureHistoryData(Window* windowPtr, bool redirectToChildren) : 
				WindowPtr(windowPtr),
				RedirectToChildren(redirectToChildren)
			{}
		};
		struct CaptureData
		{
			Window* WindowPtr{ nullptr };
			bool RedirectToChildren{ false };
			std::vector<CaptureHistoryData> PrevCaptured;
		}m_capture;

		std::map<API::NativeWindowHandle, FormData> m_windowNativeRegistry;
		std::set<Window*> m_windowRegistry;

		MenuItemReactor* m_rootMenuItemReactor{ nullptr };
		bool m_keyboardCaptured{ false };
	};
}

#endif