/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#include "btpch.h"
#include "DockPanel.h"

namespace Berta
{
	void DockPanelReactor::Update(Graphics& graphics)
	{
		auto window = m_control->Handle();
		auto clientRect = window->ClientSize.ToRectangle();
		graphics.FillRectangle(clientRect, window->Appearance->SelectionHighlightColor);
		graphics.DrawRectangle(clientRect, window->Appearance->SelectionBorderHighlightColor, 3.0f);
	}

	DockPanel::DockPanel(Window* parent, const Rectangle& rectangle, bool visible)
	{
		Create(parent, true, rectangle, visible);

#if BT_DEBUG
		m_handle->Name = "DockPanel";
#endif
	}
	DockPanel::DockPanel(Window* parent, bool isUnscaleRect, const Rectangle& rectangle, bool visible)
	{
		Create(parent, isUnscaleRect, rectangle, visible);

#if BT_DEBUG
		m_handle->Name = "DockPanel";
#endif
	}
}