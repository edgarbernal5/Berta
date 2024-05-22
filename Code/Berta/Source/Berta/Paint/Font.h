/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#ifndef BT_FONT_HEADER
#define BT_FONT_HEADER

#include <cstdint>
#include <string>
#ifdef BT_PLATFORM_WINDOWS
#include <windef.h>
#endif

namespace Berta
{
	struct FontStyle;
	struct FontInfo;

	struct NativeFont
	{
#ifdef BT_PLATFORM_WINDOWS
		HFONT Handle{ nullptr };
#endif
	};

	struct Font
	{
		NativeFont Native;
	};

	struct FontStyle
	{
		uint32_t Weight{ 400 };
		bool Italic{ false };
		bool Underline{ false };
	};

	struct FontInfo
	{
		std::string Family;
		FontStyle Style;
	};
}

#endif