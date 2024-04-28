/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#ifndef BT_API_HEADER
#define BT_API_HEADER

#include "Berta/Core/Base.h"
#include "Berta/Core/BasicTypes.h"

namespace Berta::API
{
	struct NativeWindowHandle
	{
		HWND Handle;
	};
	NativeWindowHandle Create_Window(const Rectangle& rectangle);
}

#endif