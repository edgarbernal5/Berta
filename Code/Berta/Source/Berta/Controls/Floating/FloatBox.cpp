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
		Rectangle rect{ 0,0,window->Size.Width,window->Size.Height };
		if (m_scrollBar)
		{
			rect.Width -= m_scrollBar->Handle()->Size.Width - 1;
		}
		graphics.DrawRectangle(rect, window->Appereance->BoxBackground, true);
		
		if (m_interactionData)
		{
			auto& items = m_interactionData->m_items;
			auto& visibleItemsCount = (std::min)(m_interactionData->m_items.size(), m_interactionData->m_maxItemsToDisplay);
			auto textItemHeight = graphics.GetTextExtent().Height;

			auto itemHeight = static_cast<uint32_t>(window->Appereance->ComboBoxItemHeight * window->DPIScaleFactor);

			for (size_t i = 0; i < visibleItemsCount; i++)
			{
				auto offsetIndex = m_state.m_offset + i;
				if (m_state.m_index == offsetIndex)
				{
					graphics.DrawRectangle({ 1, 1 + static_cast<int>(i * itemHeight), rect.Width - 2,itemHeight }, m_control->Handle()->Appereance->HighlightColor, true);
					graphics.DrawString({ 3, ((static_cast<int>(itemHeight - textItemHeight) >> 1) + 1) + static_cast<int>(i * itemHeight) }, items[offsetIndex], m_control->Handle()->Appereance->HighlightTextColor);
				}
				else
				{
					graphics.DrawString({ 3, ((static_cast<int>(itemHeight - textItemHeight) >> 1) + 1) + static_cast<int>(i * itemHeight) }, items[offsetIndex], m_control->Handle()->Appereance->Foreground);
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
			
			m_state.m_index = m_state.m_offset + index;

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

	void FloatBoxReactor::SetState(GUI::InteractionData& selection)
	{
		m_interactionData = &selection;
		m_state.m_index = selection.m_selectedIndex;
		selection.m_isSelected = false;

		//m_scrollBar
		UpdateScrollBar();
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
		return (point.X > 0 && point.X < static_cast<int>(m_control->Handle()->Size.Width) - 1 &&
			point.Y > 0 && point.Y < static_cast<int>(m_control->Handle()->Size.Height) - 2);
	}

	void FloatBoxReactor::UpdateScrollBar()
	{
		bool needScrollBar = m_interactionData->m_items.size() > m_interactionData->m_maxItemsToDisplay;
		if (!needScrollBar && m_scrollBar)
		{
			m_scrollBar.reset();
			m_state.m_offset = 0;
		}

		if (!needScrollBar && !m_scrollBar)
			return;

		auto window = m_control->Handle();
		auto scrollSize = static_cast<uint32_t>(window->Appereance->ScrollBarSize * window->DPIScaleFactor);
		Rectangle rect{ window->Size.Width - scrollSize - 1,0,scrollSize, window->Size.Height };
		if (!m_scrollBar)
		{
			m_scrollBar = std::make_unique<ScrollBar>(window, rect);
			m_scrollBar->GetEvents().ValueChanged.Connect([this](const ArgScrollBar& args) 
				{
					m_state.m_offset = args.Value;
					//BT_CORE_DEBUG << "m_state.m_offset =" << m_state.m_offset <<std::endl;
					
					m_control->Handle()->Renderer.Update();
					GUI::RefreshWindow(m_control->Handle());
				});
		}

		auto delta = m_interactionData->m_items.size() - m_interactionData->m_maxItemsToDisplay;
		m_scrollBar->SetMinMax(0, delta);
	}

	FloatBox::FloatBox(Window* parent, const Rectangle& rectangle)
	{
		Create(parent, rectangle, { false, false, false, false, true, false });
		GUI::MakeWindowActive(m_handle, false);
#if BT_DEBUG
		SetDebugName("Float box");
#endif
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