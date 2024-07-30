/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#include "btpch.h"
#include "TabBar.h"

#include "Berta/GUI/Interface.h"
#include "Panel.h"

namespace Berta
{
	void PanelReactor::Init(ControlBase& control)
	{
	}

	void PanelReactor::Update(Graphics& graphics)
	{
	}

	Panel::Panel(Window* parent, const Rectangle& rectangle)
	{
		Create(parent, true, rectangle);
	}
}
