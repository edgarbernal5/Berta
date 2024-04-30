/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#ifndef BT_BASIC_TYPES_HEADER
#define BT_BASIC_TYPES_HEADER

#include <cstdint>

namespace Berta
{
	struct Rectangle
	{
		int x;
		int y;
		uint32_t width;
		uint32_t height;
	};

	struct Size
	{
		uint32_t Width;
		uint32_t Height;
	};

	struct WindowStyle
	{
		bool Minimize{ true };
		bool Maximize{ true };
		bool Sizable{ true };
	};
}

#endif