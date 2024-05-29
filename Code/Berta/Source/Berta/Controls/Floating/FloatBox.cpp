/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#include "btpch.h"
#include "FloatBox.h"

namespace Berta
{
	FloatBoxReactor::~FloatBoxReactor()
	{
	}

	void FloatBoxReactor::Init(ControlBase& control)
	{
		m_control = reinterpret_cast<FloatBox*>(&control);
	}

	void FloatBoxReactor::Update(Graphics& graphics)
	{
		graphics.DrawRectangle(GUI::GetBoxBackgroundColor(*m_control), true);
		if (m_control->m_items)
		{
			auto& items = *m_control->m_items;
			for (size_t i = 0; i < items.size(); i++)
			{
				graphics.DrawString({ 2, ((static_cast<int>(30 - graphics.GetTextExtent().Height) >> 1) + 1) + static_cast<int>(i * 30) }, items[i], m_control->Handle()->Appereance->Foreground);
			}
		}
	}

	FloatBox::FloatBox(Window* parent, const Rectangle& rectangle)
	{
		Create(parent, rectangle, { false, false, false, false, true, false });
		GUI::MakeWindowActive(m_handle, false);
	}

	FloatBox::~FloatBox()
	{
		::ReleaseCapture();
	}
}