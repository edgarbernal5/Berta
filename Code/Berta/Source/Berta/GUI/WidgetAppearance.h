/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#ifndef BT_WIDGET_APPEARANCE_HEADER
#define BT_WIDGET_APPEARANCE_HEADER

#include "Berta/Core/BasicTypes.h"
#include "Berta/Core/Colors.h"

namespace Berta
{
	struct WidgetAppearance
	{
		Color Background{ Colors::Light_Background };
		Color Foreground{ Colors::Light_Foreground };
	};
}

#endif