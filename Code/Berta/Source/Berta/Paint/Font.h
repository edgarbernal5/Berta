/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#ifndef BT_FONT_HEADER
#define BT_FONT_HEADER

#include <cstdint>
#include <string>

namespace Berta
{
	struct FontStyle;
	struct FontInfo;

	struct NativeFont
	{
#ifdef BT_PLATFORM_WINDOWS
		HFONT m_hFont{ nullptr };
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