/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#ifndef BT_ENTRY_POINT_HEADER
#define BT_ENTRY_POINT_HEADER

#include "Berta/Core/Base.h"
#include "Berta/Core/Foundation.h"

#ifndef BT_PLATFORM_WINDOWS
#define BT_PLATFORM_WINDOWS
#endif

#ifdef BT_PLATFORM_WINDOWS

namespace Berta
{
	namespace
	{
		//auto& foundation = Foundation::Instance();
	}
}

#else

#error Berta only supports Windows!

#endif

#endif