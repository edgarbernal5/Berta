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

	Panel::Panel(Window* parent, const Rectangle& rectangle, bool visible)
	{
		Create(parent, true, rectangle, visible, true);

#if BT_DEBUG
		m_handle->Name = "Panel";
#endif
	}
	Panel::Panel(Window* parent, bool isUnscaleRect, const Rectangle& rectangle, bool visible)
	{
		Create(parent, isUnscaleRect, rectangle, visible, true);

#if BT_DEBUG
		m_handle->Name = "Panel";
#endif
	}
}
