/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#ifndef BT_CONTROL_APPEARANCE_HEADER
#define BT_CONTROL_APPEARANCE_HEADER

#include "Berta/Core/BasicTypes.h"
#include "Berta/Core/Colors.h"

namespace Berta
{
	struct ControlAppearance
	{
		Color Background{ Colors::Light_Background };
		Color Foreground{ Colors::Light_Foreground };

		Color BoxBackground{ Colors::Light_BoxBackground };
		Color BoxBorderColor{ Colors::Light_BoxBorderColor };

		Color HighlightColor{ Colors::Light_HighlightColor };
		Color HighlightTextColor{ Colors::Light_HighlightTextColor };

		uint32_t ComboBoxItemHeight = 24;
	};
}

#endif