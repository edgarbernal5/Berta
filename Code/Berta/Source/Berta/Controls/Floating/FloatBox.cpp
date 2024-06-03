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
			auto textItemHeight = graphics.GetTextExtent().Height;
			for (size_t i = 0; i < items.size(); i++)
			{
				if (m_index == i)
				{
					graphics.DrawRectangle({1, 1 + static_cast<int>(i * m_control->GetAppearance().ComboBoxItemHeight), m_control->Handle()->Size.Width-2, m_control->GetAppearance().ComboBoxItemHeight }, m_control->Handle()->Appereance->HighlightColor, true);
					graphics.DrawString({ 2, ((static_cast<int>(m_control->GetAppearance().ComboBoxItemHeight - textItemHeight) >> 1) + 1) + static_cast<int>(i * m_control->GetAppearance().ComboBoxItemHeight) }, items[i], m_control->Handle()->Appereance->HighlightTextColor);
				}
				else
				{
					graphics.DrawString({ 2, ((static_cast<int>(m_control->GetAppearance().ComboBoxItemHeight - textItemHeight) >> 1) + 1) + static_cast<int>(i * m_control->GetAppearance().ComboBoxItemHeight) }, items[i], m_control->Handle()->Appereance->Foreground);
				}
			}
		}

		graphics.DrawRectangle(m_control->GetAppearance().BoxBorderColor, false);
	}

	void FloatBoxReactor::MouseMove(Graphics& graphics, const ArgMouse& args)
	{
		auto& items = *m_control->m_items;
		if (args.Position.X > 0 && args.Position.X < m_control->Handle()->Size.Width - 1 &&
			args.Position.Y > 0 && args.Position.Y < m_control->Handle()->Size.Height - 2)
		{
			auto index = args.Position.Y / m_control->GetAppearance().ComboBoxItemHeight;

			m_index = index;

			Update(graphics);
			GUI::UpdateDeferred(*m_control);
		}
	}

	void FloatBoxReactor::MouseUp(Graphics& graphics, const ArgMouse& args)
	{
		if (m_control->m_ignoreFirstMouseUp)
		{
			m_control->m_ignoreFirstMouseUp = false;
			return;
		}

		if (args.Position.X > 0 && args.Position.X < m_control->Handle()->Size.Width - 1 &&
			args.Position.Y > 0 && args.Position.Y < m_control->Handle()->Size.Height - 2)
		{
			*m_control->m_selectedIndex = m_index;

			//update combo box?
			GUI::UpdateDeferred(*m_control);
		}

		m_control->Dispose();
	}

	void FloatBoxReactor::SetIndex(int index)
	{
		m_index = index;
	}

	FloatBox::FloatBox(Window* parent, const Rectangle& rectangle)
	{
		Create(parent, rectangle, { false, false, false, false, true, false });
		GUI::MakeWindowActive(m_handle, false);

		m_ignoreFirstMouseUp = true;
	}

	FloatBox::~FloatBox()
	{
		//GUI::ReleaseCapture(this->Handle());
	}
}