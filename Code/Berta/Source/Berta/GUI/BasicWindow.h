/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#ifndef BT_BASIC_WINDOW_HEADER
#define BT_BASIC_WINDOW_HEADER

#include <string>
#include "Berta/Core/BasicTypes.h"

namespace Berta
{
	struct BasicWindow
	{
		std::string Title;
		bool Visible;
		Size Size;
	};
}

#endif