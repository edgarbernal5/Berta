/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#ifndef BT_INTERFACE_HEADER
#define BT_INTERFACE_HEADER

#include "Berta/Core/BasicTypes.h"
#include "Berta/GUI/Window.h"
#include "Berta/Platform/Windows/Messages.h"

namespace Berta
{
	class ControlBase;
	class ControlReactor;
	struct ControlEvents;
	class MenuItemReactor;

	namespace GUI
	{
		Window* CreateForm(Window* parent, const Rectangle& rectangle, const FormStyle& windowStyle, ControlBase* control);
		Window* CreateControl(Window* parent, bool isUnscaleRect, const Rectangle& rectangle, ControlBase* control, bool isPanel);

		void CaptionWindow(Window* window, const std::wstring& caption);
		std::wstring CaptionWindow(Window* window);
		void DisposeWindow(Window* window);
		void ShowWindow(Window* window, bool visible);
		void UpdateWindow(Window* window);
		void EnableWindow(Window* window, bool isEnabled);
		bool EnableWindow(Window* window);

		void ResizeWindow(Window* window, const Size& newSize);
		Size SizeWindow(Window* window);
		void MoveWindow(Window* window, const Rectangle& newRect);

		void MakeWindowActive(Window* window, bool active);

		void Capture(Window* window, bool redirectToChildren = false);
		void ReleaseCapture(Window* window);

		void InitRendererReactor(ControlBase* window, ControlReactor& controlReactor);
		void SetEvents(Window* window, std::shared_ptr<ControlEvents> events);
		void SetAppearance(Window* window, std::shared_ptr<ControlAppearance> controlAppearance);

		Point GetAbsolutePosition(Window* window);
		Point GetLocalPosition(Window* window);
		Point GetMousePositionToWindow(Window* window);

		void UpdateTree(Window* window);
		void MarkAsUpdated(Window* window);

		void ChangeCursor(Window* window, Cursor newCursor);
		Cursor GetCursor(Window* window);

		Rectangle GetCenteredOnScreen(uint32_t width, uint32_t height);
		Rectangle GetCenteredOnScreen(const Size& size);

		Point GetPointClientToScreen(Window *window, const Point& point);
		Point GetPointScreenToClient(Window *window, const Point& point);

		void SendCustomMessage(Window* window, std::function<void()> body);

		void SetMenu(MenuItemReactor* rootMenuItemWindow);

		void DisposeMenu();
		void DisposeMenu(MenuItemReactor* rootReactor);
	}
}

#endif