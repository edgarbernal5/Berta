/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#ifndef BT_WINDOW_API_HEADER
#define BT_WINDOW_API_HEADER

#include <string>
#include "Berta/Core/Base.h"
#include "Berta/Core/BasicTypes.h"

#include "Berta/Platform/Windows/Messages.h"

namespace Berta
{
	namespace API
	{
		struct NativeWindowHandle
		{
#ifdef BT_PLATFORM_WINDOWS
			HWND Handle{ nullptr };

			constexpr bool operator<(const NativeWindowHandle& other) const
			{
				return Handle < other.Handle;
			}

			constexpr bool operator==(const NativeWindowHandle& other) const
			{
				return Handle == other.Handle;
			}

			constexpr bool operator!=(const NativeWindowHandle& other) const
			{
				return Handle != other.Handle;
			}
#endif
			NativeWindowHandle(const NativeWindowHandle&) = default;
			NativeWindowHandle& operator=(const NativeWindowHandle&) = default;
		};

		struct NativeWindowResult
		{
			NativeWindowHandle WindowHandle;
			Size ClientSize;
			uint32_t DPI;
		};

		struct NativeCursor
		{
#ifdef BT_PLATFORM_WINDOWS
			HCURSOR Handle{ nullptr };
			Cursor CursorType{ Cursor::Default };
#endif
		};

		NativeWindowResult CreateNativeWindow(NativeWindowHandle parentHandle, const Rectangle& rectangle, const FormStyle& windowStyle, bool isNested);
		void CaptionNativeWindow(NativeWindowHandle nativeHandle, const std::wstring& caption);
		std::wstring GetCaptionNativeWindow(NativeWindowHandle nativeHandle);
		void DestroyNativeWindow(NativeWindowHandle nativeHandle);
		void ShowNativeWindow(NativeWindowHandle nativeHandle, bool visible, bool active);
		void RefreshWindow(NativeWindowHandle nativeHandle);
		void EnableWindow(NativeWindowHandle nativeHandle, bool isEnabled);
		NativeWindowHandle GetParentWindow(NativeWindowHandle nativeHandle);
		void MoveWindow(NativeWindowHandle nativeHandle, const Rectangle& newMove);
		void ResizeWindow(NativeWindowHandle nativeHandle, const Size& newSize);
		Point GetWindowPosition(NativeWindowHandle nativeHandle);

		void CaptureWindow(NativeWindowHandle nativeHandle, bool capture);
		uint32_t GetNativeWindowDPI(NativeWindowHandle nativeHandle);

		bool ChangeCursor(NativeWindowHandle nativeHandle, Cursor newCursor, NativeCursor& nativeCursor);
		Size GetPrimaryMonitorSize();

		Point GetPointClientToScreen(NativeWindowHandle nativeHandle, const Point& point);
		Point GetPointScreenToClient(NativeWindowHandle nativeHandle, const Point& point);
		void SendCustomMessage(API::NativeWindowHandle nativeHandle, std::function<void()> body);

		void ResizeChildWindow(NativeWindowHandle nativeHandle, Point position, Size size);
		Point GetScreenMousePosition();
	}
}

#endif