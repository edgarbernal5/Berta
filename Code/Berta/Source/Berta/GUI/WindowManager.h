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

#include "ControlEvents.h"
#include "Berta/API/WindowAPI.h"
#include "Berta/Paint/Graphics.h"

namespace Berta
{
	struct Window;
	class ControlBase;

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
		bool Caption(Window* window, const std::wstring& caption);
		Window* CreateForm(Window* parent, bool isUnscaleRect, Rectangle rectangle, const FormStyle& formStyle, bool isNested, ControlBase* control, bool isRenderForm);
		Window* CreateControl(Window* parent, bool isUnscaleRect, const Rectangle& rectangle, ControlBase* control, bool isPanel);
		void Destroy(Window* window);
		void Dispose(Window* window);
		void Remove(Window* window);
		void Refresh(Window* window);
		Window* Get(API::NativeWindowHandle nativeWindowHandle) const;
		FormData* GetFormData(API::NativeWindowHandle nativeWindowHandle);
		bool Exists(Window* window) const;
		uint32_t NativeWindowCount();

		void Capture(Window* window, bool redirectToChildren);
		void ReleaseCapture(Window* window);
		Window* GetCaptureWindow() const;

		Window* Find(Window* window, const Point& point);
		void UpdateTree(Window* window, bool now = false);
		void Show(Window* window, bool visible);

		bool Resize(Window* window, const Size& newSize, bool resizeForm = true);
		bool Move(Window* window, const Rectangle& newRect, bool forceRepaint = true);
		bool Move(Window* window, Point newPosition, bool forceRepaint = true);
		void Update(Window* window, bool redraw, const Rectangle* updateArea = nullptr);

		void ChangeDPI(Window* window, uint32_t newDPI, const API::NativeWindowHandle& nativeWindowHandle);
		void ChangeCursor(Window* window, Cursor newCursor);
		Cursor GetCursor(Window* window);

		Point GetWindowRootPosition(Window* window);
		Point GetWindowPosition(Window* window);

		void SetParent(Window* window, Window* newParent);

		void GetNativeWindows(std::vector<API::NativeWindowHandle>& windows);

		void EnterSizeMove(Window* window);
		void ExitSizeMove(Window* window);

		void Focus(Window* window, ArgFocus::Reason reason);
	private:

		void UpdateInternal(Window* window, bool redraw, const Rectangle* updateArea = nullptr);
		bool IsPointOnWindow(Window* window, const Point& point);
		Window* FindInTree(Window* window, const Point& point);
		void DestroyInternal(Window* window);
		
		void SetParentInternal(Window* window, Window* newParent, const Point& deltaPosition);
		void MoveInternal(Window* window, const Point& delta, bool forceRepaint);
		void ShowInternal(Window* window, bool visible);
		
		void EnterSizeMoveInternal(Window* window);
		void ExitSizeMoveInternal(Window* window);

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

		bool m_keyboardCaptured{ false };
	};
}

#endif