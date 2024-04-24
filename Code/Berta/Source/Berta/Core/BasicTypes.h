/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#ifndef BERTA_BASIC_TYPES_HEADER
#define BERTA_BASIC_TYPES_HEADER

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
}

#endif