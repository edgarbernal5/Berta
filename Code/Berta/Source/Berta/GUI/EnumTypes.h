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
		enum {
			Backspace = 0x8,
			Enter= 0x0D,
			Escape = 0x1b,
			ArrowLeft = 0x25,
			ArrowUp,
			ArrowRight,
			ArrowDown,
			Delete=0x7F,
		};
	};
}

#endif