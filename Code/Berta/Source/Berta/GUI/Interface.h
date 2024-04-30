/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#ifndef BT_INTERFACE_HEADER
#define BT_INTERFACE_HEADER

#include "Berta/Core/BasicTypes.h"
#include "Berta/GUI/BasicWindow.h"

namespace Berta::GUI
{
	BasicWindow* CreateBasicWindow(const Rectangle& rectangle, const WindowStyle& windowStyle);
	BasicWindow* CreateWidget(const Rectangle& rectangle);

	void ShowBasicWindow(BasicWindow* basicWindow, bool visible);
}

#endif