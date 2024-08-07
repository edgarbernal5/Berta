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

	Panel::Panel(Window* parent, const Rectangle& rectangle, bool visible)
	{
		Create(parent, true, rectangle, visible);
	}

	void Panel::Create(Window* parent, bool isUnscaleRect, const Rectangle& rectangle, bool visible)
	{
		m_handle = GUI::CreateControl(parent, isUnscaleRect, rectangle, this, true);
		m_appearance = std::make_shared<AppearanceType>();
		m_events = std::make_shared<EventsType>();
		GUI::SetEvents(m_handle, m_events);
		GUI::SetAppearance(m_handle, m_appearance);

		if (visible)
		{
			GUI::ShowWindow(m_handle, true);
		}
	}
}
