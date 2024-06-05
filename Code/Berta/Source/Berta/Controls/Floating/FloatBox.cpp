/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#include "btpch.h"
#include "FloatBox.h"
#include "Berta/GUI/EnumTypes.h"

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
		auto window = m_control->Handle();
		graphics.DrawRectangle(window->Appereance->BoxBackground, true);
		
		if (m_interactionData)
		{
			auto& items = m_interactionData->m_items;
			auto textItemHeight = graphics.GetTextExtent().Height;

			auto itemHeight = static_cast<uint32_t>(window->Appereance->ComboBoxItemHeight * window->DPIScaleFactor);

			for (size_t i = 0; i < items.size(); i++)
			{
				if (m_state.m_index == i)
				{
					graphics.DrawRectangle({ 1, 1 + static_cast<int>(i * itemHeight), m_control->Handle()->Size.Width - 2,itemHeight }, m_control->Handle()->Appereance->HighlightColor, true);
					graphics.DrawString({ 2, ((static_cast<int>(itemHeight - textItemHeight) >> 1) + 1) + static_cast<int>(i * itemHeight) }, items[i], m_control->Handle()->Appereance->HighlightTextColor);
				}
				else
				{
					graphics.DrawString({ 2, ((static_cast<int>(itemHeight - textItemHeight) >> 1) + 1) + static_cast<int>(i * itemHeight) }, items[i], m_control->Handle()->Appereance->Foreground);
				}
			}
		}
		

		graphics.DrawRectangle(m_control->GetAppearance().BoxBorderColor, false);
	}

	void FloatBoxReactor::MouseMove(Graphics& graphics, const ArgMouse& args)
	{
		if (IsInside(args.Position))
		{
			auto window = m_control->Handle();
			auto itemHeight = static_cast<uint32_t>(window->Appereance->ComboBoxItemHeight * window->DPIScaleFactor);
			auto index = (args.Position.Y - 1) / itemHeight;
			
			m_state.m_index = index;

			Update(graphics);
			GUI::UpdateDeferred(*m_control);
		}
	}

	void FloatBoxReactor::MouseUp(Graphics& graphics, const ArgMouse& args)
	{
		if (m_ignoreFirstMouseUp)
		{
			m_ignoreFirstMouseUp = false;
			return;
		}

		if (IsInside(args.Position))
		{
			m_interactionData->m_isSelected = true;
		}

		m_control->Dispose();
	}

	void FloatBoxReactor::KeyPressed(Graphics& graphics, const ArgKeyboard& args)
	{
	}


	bool FloatBoxReactor::MoveSelectedItem(int direction)
	{
		int newIndex = (std::max)(0, (std::min)(m_state.m_index + direction, (int)(m_interactionData->m_items.size()) - 1));
		if (m_state.m_index != newIndex && !m_interactionData->m_items.empty())
		{
			m_state.m_index = newIndex;

			m_control->Handle()->Renderer.Update();
			GUI::RefreshWindow(m_control->Handle());
			return true;
		}
		return false;
	}

	bool FloatBoxReactor::IsInside(const Point& point)
	{
		return (point.X > 0 && point.X < m_control->Handle()->Size.Width - 1 &&
			point.Y > 0 && point.Y < m_control->Handle()->Size.Height - 2);
	}

	FloatBox::FloatBox(Window* parent, const Rectangle& rectangle)
	{
		Create(parent, rectangle, { false, false, false, false, true, false });
		GUI::MakeWindowActive(m_handle, false);
	}

	FloatBox::~FloatBox()
	{
	}

	bool FloatBox::OnKeyPressed(const ArgKeyboard& args)
	{
		bool redraw = false;
		//m_reactor.KeyPressed(m_handle->Renderer.GetGraphics(), args);
		return redraw;
	}

	bool FloatBox::MoveSelectedItem(int direction)
	{
		return m_reactor.MoveSelectedItem(direction);
	}
}