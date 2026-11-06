/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#ifndef BT_GUI_ENUM_TYPES_HEADER
#define BT_GUI_ENUM_TYPES_HEADER

#include "Berta/Core/Colors.h"

namespace Berta
{
	enum class DockPosition
	{
		Up,
		Down,
		Left,
		Right,
		Tab
	};

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
	
	// Tokens semánticos globales
	struct ThemePalette
	{
		Color BaseBackground { Colors::Light_Background };       // ej. rgb(212, 208, 200) para tu Light Theme
		Color Surface { Colors::Light_BoxBackground };              // Fondo de paneles y cajas
		Color PrimaryAction { Colors::Light_HighlightColor };        // Color principal (ej. azul de selección)
        
		Color TextPrimary { Colors::Light_Foreground };
		Color TextSecondary { Colors::Light_Foreground2nd };
		Color TextDisabled;

		Color Border{ Colors::Light_BoxBorderColor };
		Color BorderHighlight{ Colors::Light_BoxBorderColor };
        
		Color Error{ 0xFF0000FF };                // ej. Red{ 0xFF0000FF }
	};
}

#endif