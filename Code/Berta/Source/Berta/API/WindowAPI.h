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

namespace Berta::API
{
	struct NativeWindowHandle
	{
#ifdef BT_PLATFORM_WINDOWS
		HWND Handle;
#endif
	};

	NativeWindowHandle CreateNativeWindow(const Rectangle& rectangle, const WindowStyle& windowStyle);
	void CaptionNativeWindow(NativeWindowHandle nativeHandle, const std::wstring& caption);
	void DestroyNativeWindow(NativeWindowHandle nativeHandle);
	void ShowNativeWindow(NativeWindowHandle nativeHandle, bool visible);
}

#endif