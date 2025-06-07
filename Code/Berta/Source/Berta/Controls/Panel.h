/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#ifndef BT_PANEL_HEADER
#define BT_PANEL_HEADER

#include "Berta/GUI/Window.h"
#include "Berta/GUI/Control.h"

#include <string>
#include <vector>

namespace Berta
{
	class PanelReactor : public ControlReactor
	{
	public:

	private:
	};

	class Panel : public Control<ControlReactor>
	{
	public:
		Panel() = default;
		Panel(Window* parent, const Rectangle& rectangle = {}, bool visible = true);
		Panel(Window* parent, bool isUnscaleRect, const Rectangle& rectangle = {}, bool visible = true);
	};
}

#endif
