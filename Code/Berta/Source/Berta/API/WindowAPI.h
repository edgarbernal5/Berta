/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#ifndef BT_WINDOW_API_HEADER
#define BT_WINDOW_API_HEADER

#include "Berta/Core/Base.h"
#include "Berta/Core/BasicTypes.h"

namespace Berta::API
{
	struct NativeWindowHandle
	{
		HWND Handle;
	};

	NativeWindowHandle CreateNativeWindow(const Rectangle& rectangle);
	void DestroyNativeWindow(NativeWindowHandle nativeHandle);
	void ShowNativeWindow(NativeWindowHandle nativeHandle, bool visible);
}

#endif