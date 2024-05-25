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

		NativeWindowResult CreateNativeWindow(NativeWindowHandle parentHandle, const Rectangle& rectangle, const FormStyle& windowStyle);
		void CaptionNativeWindow(NativeWindowHandle nativeHandle, const std::wstring& caption);
		std::wstring GetCaptionNativeWindow(NativeWindowHandle nativeHandle);
		void DestroyNativeWindow(NativeWindowHandle nativeHandle);
		void ShowNativeWindow(NativeWindowHandle nativeHandle, bool visible);
		void RefreshWindow(NativeWindowHandle nativeHandle);

		void CaptureWindow(NativeWindowHandle nativeHandle, bool capture);
		uint32_t GetNativeWindowDPI(NativeWindowHandle nativeHandle);

		bool ChangeCursor(NativeWindowHandle nativeHandle, Cursor newCursor, NativeCursor& nativeCursor);
		Size GetPrimaryMonitorSize();

		Point GetPointClientToScreen(NativeWindowHandle nativeHandle, const Point& point);
	}
}

#endif