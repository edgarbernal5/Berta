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
		Color Foreground2nd{ Colors::Light_Foreground2nd };

		//Highlight = Hovered
		Color ButtonBackground{ Colors::Light_ButtonBackground };
		Color ButtonHighlightBackground{ Colors::Light_ButtonHightlighBackground };
		Color ButtonPressedBackground{ Colors::Light_ButtonPressedBackground };

		Color BoxBackground{ Colors::Light_BoxBackground };
		Color BoxBorderColor{ Colors::Light_BoxBorderColor };

		Color Red{ 0xFF0000FF };
		
		//Color ButtonTest1{ 0x7D99B2 };
		//Color ButtonTest2{ 0x3E4659 };
		
		Color ButtonTest1{ 0xFFD6DFE3 };
		Color ButtonTest2{ 0xFFB7BEC2 };

		Color ButtonDisabledBackground{ Colors::Light_ButtonDisabledBackground };
		Color BoxBorderDisabledColor{ Colors::Light_BoxBorderDisabledColor };

		Color HighlightColor{ Colors::Light_HighlightColor };
		Color HighlightBorderColor{ Colors::Light_HighlightBorderColor };
		Color HighlightTextColor{ Colors::Light_HighlightTextColor };

		Color BoxBorderHighlightColor{ Colors::Light_BoxBorderHighlightColor };
		Color BoxHightlightBackground{ Colors::Light_BoxHightlightBackground };
		Color BoxPressedBackground{ Colors::Light_BoxPressedBackground };

		Color ScrollBarBackground{ Colors::Light_ScrollBarBackground };
		Color MenuBackground{ Colors::Light_MenuBackground };

		Color ItemCollectionHightlightBackground{ Colors::Light_ItemCollectionHightlightBackground };

		Color SelectionHighlightColor{ Colors::Light_SelectionHighlightColor };
		Color SelectionBorderHighlightColor{ Colors::Light_SelectionBorderHighlightColor };

		uint32_t CheckboxHeight = 12;
		uint32_t ScrollBarSize = 18;
		uint32_t SmallIconSize = 16;

	};
}

#endif