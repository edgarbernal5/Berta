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
#include "Berta/Paint/DrawBatch.h"

namespace Berta
{
	struct Window;
	class MenuItemReactor;
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
		Window* CreateForm(Window* parent, bool isUnscaleRect, const Rectangle& rectangle, const FormStyle& formStyle, bool isNested, ControlBase* control, bool isRenderForm);
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

		Window* Find(Window* window, const Point& point);
		void UpdateTree(Window* window, bool now = false);
		void Map(Window* window, const Rectangle* areaToUpdate);
		void Show(Window* window, bool visible);

		bool Resize(Window* window, const Size& newSize, bool resizeForm = true);
		bool Move(Window* window, const Rectangle& newRect, bool forceRepaint = true);
		bool Move(Window* window, const Point& newPosition, bool forceRepaint = true);
		void Update(Window* window);
		void Paint(Window* window, bool doUpdate);

		void ChangeDPI(Window* window, uint32_t newDPI, const API::NativeWindowHandle& nativeWindowHandle);
		void ChangeCursor(Window* window, Cursor newCursor);
		Cursor GetCursor(Window* window);

		Point GetAbsolutePosition(Window* window);
		Point GetAbsoluteRootPosition(Window* window);
		Point GetLocalPosition(Window* window);

		void SetParent(Window* window, Window* newParent);

		void SetMenu(MenuItemReactor* rootMenuItemWindow);
		MenuItemReactor* GetMenu();

		void DisposeMenu();
		void DisposeMenu(MenuItemReactor* rootReactor);

		void TryAddWindowToBatch(Window* window, const DrawOperation& operation = DrawOperation::NeedUpdate | DrawOperation::NeedMap);

		void GetNativeWindows(std::vector<API::NativeWindowHandle>& windows);
	private:
		void AddWindowToBatch(Window* window, const Rectangle& areaToUpdate, const DrawOperation& operation);
		void AddWindowToBatch(DrawBatch* batch, Window* window, const Rectangle& areaToUpdate, const DrawOperation& operation);
		void TryAddWindowToBatchInternal(Window* window, const Rectangle& containerRectangle, const Point& parentPosition, const DrawOperation& operation);

		bool GetIntersectionClipRect(Window* window, Rectangle& result);
		bool IsPointOnWindow(Window* window, const Point& point);
		Window* FindInTree(Window* window, const Point& point);
		void DestroyInternal(Window* window);
		void UpdateTreeInternal(Window* window, Graphics& rootGraphics, bool now, const Point& parentPosition = {}, const Rectangle& parentRectangle = {});
		void PaintInternal(Window* window, Graphics& rootGraphics, bool doUpdate, const Point& parentPosition = {}, const Rectangle& parentRectangle = {});
		
		void SetParentInternal(Window* window, Window* newParent, const Point& deltaPosition);
		void MoveInternal(Window* window, const Point& delta, bool forceRepaint);
		void ShowInternal(Window* window, bool visible);

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