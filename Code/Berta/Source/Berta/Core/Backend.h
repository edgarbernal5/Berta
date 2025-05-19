/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#ifndef BT_BACKEND_HEADER
#define BT_BACKEND_HEADER

#ifdef BT_PLATFORM_WINDOWS

//#if !defined(BT_BACKEND_GDI) && !defined(BT_BACKEND_D2D)
//#error Berta supports either GDI or Direct2D!
//#endif

namespace Berta
{
	enum class Backend
	{
#ifdef BT_PLATFORM_WINDOWS
		GDI,
		D2D
#else
		Other
#endif
	};
}

#endif

#endif