/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#ifndef BT_COLORS_HEADER
#define BT_COLORS_HEADER

#include "Berta/Core/BasicTypes.h"

namespace Berta
{
	namespace Colors
	{
		static Color Light_Background{ 0xFFC8D0D4 }; //0xC8D0D4. //0xC6CED2
		static Color Light_Foreground{ 0xFF3C3C3C }; //Primary text
		static Color Light_Foreground2nd{ 0xFF606060 }; //Primary text

		static Color Light_ButtonBackground{ 0xFFBCC3C7 };
		static Color Light_ButtonHightlighBackground{ 0xFFCBD3D6 };
		static Color Light_ButtonPressedBackground{ 0xFF9FA6A8 };

		static Color Light_ButtonDisabledBackground{ 0xFFD2D9DE };
		static Color Light_BoxBorderDisabledColor{ 0xFF92989C };

		static Color Light_BoxBackground{ 0xFFFFFFFF };
		static Color Light_BoxBorderColor{ 0xFF808080 };

		static Color Light_HighlightColor{ 0xFFDAC7BC }; //0xBE9270 
		static Color Light_HighlightBorderColor{ 0xFFBB937F }; //0xBE9270 
		static Color Light_HighlightTextColor{ 0xFFFFFFFF };

		static Color Light_BoxBorderHighlightColor{ 0xFF7B99A8 };
		static Color Light_BoxHightlightBackground{ 0xFFE2E6E7 };
		static Color Light_BoxPressedBackground{ 0xFFE0E8E5 };

		static Color Light_ScrollBarBackground{ 0xFFE2E6E7 }; //0x808080, 0x8E8E8F
		static Color Light_MenuBackground{ 0xFFDDE6EB };

		static Color Light_ItemCollectionHightlightBackground{ 0xFFF0E4DC };
		static Color Light_SelectionHighlightColor{ 0xFF507298 };
		static Color Light_SelectionBorderHighlightColor{ 0xFF233242 };
	}
}

#endif