/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#ifndef BT_ENUM_TYPES_HEADER
#define BT_ENUM_TYPES_HEADER

namespace Berta
{
	struct KeyboardKey
	{
		enum
		{
			Backspace = 0x8,
			Enter = 0x0D,
			Shift = 0x10,
			Control = 0x11,
			Alt = 0x12,
			Escape = 0x1b,
			Space = 0x20,
			PageUp = 0x21, PageDown,
			End = 0x23,
			Home = 0x24,
			ArrowLeft = 0x25,
			ArrowUp,
			ArrowRight,
			ArrowDown,
			Delete = 0x2E,
		};
	};
}

#endif