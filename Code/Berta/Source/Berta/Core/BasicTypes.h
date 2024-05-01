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
		int X;
		int Y;
		uint32_t Width;
		uint32_t Height;
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

	struct Color
	{

	};
}

#endif