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

		Color ButtonBackground{ Colors::Light_ButtonBackground };
		Color ButtonHighlightBackground{ Colors::Light_ButtonHightlighBackground };
		Color ButtonPressedBackground{ Colors::Light_ButtonPressedBackground };

		Color BoxBackground{ Colors::Light_BoxBackground };
		Color BoxBorderColor{ Colors::Light_BoxBorderColor };

		Color HighlightColor{ Colors::Light_HighlightColor };
		Color HighlightBorderColor{ Colors::Light_HighlightBorderColor };
		Color HighlightTextColor{ Colors::Light_HighlightTextColor };

		Color BoxBorderHighlightColor{ Colors::Light_BoxBorderHighlightColor };
		Color BoxHightlightBackground{ Colors::Light_BoxHightlightBackground };
		Color BoxPressedBackground{ Colors::Light_BoxPressedBackground };

		uint32_t ComboBoxItemHeight = 20;
		uint32_t ScrollBarSize = 16;
	};
}

#endif