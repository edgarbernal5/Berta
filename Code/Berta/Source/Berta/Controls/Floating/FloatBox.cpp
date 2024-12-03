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
			rect.Width -= m_scrollBar->Handle()->Size.Width;
		}
		graphics.DrawRectangle(rect, window->Appearance->BoxBackground, true);
		
		if (m_interactionData)
		{
			auto& items = m_interactionData->m_items;
			auto visibleItemsCount = (std::min)(m_interactionData->m_items.size(), m_interactionData->m_maxItemsToDisplay);
			auto textItemHeight = graphics.GetTextExtent().Height;

			auto itemHeight = window->ToScale(window->Appearance->ComboBoxItemHeight);

			for (size_t i = 0; i < visibleItemsCount; i++)
			{
				auto offsetIndex = m_state.m_offset + i;
				bool isSelected = m_state.m_selectedIndex == offsetIndex;
				bool isHovered = m_state.m_hoveredIndex == offsetIndex;
				Rectangle itemRect{ 2, 1 + static_cast<int>(i * itemHeight), rect.Width - 4,itemHeight };
				if (isSelected)
				{
					graphics.DrawRoundRectBox(itemRect, window->Appearance->HighlightColor, window->Appearance->HighlightBorderColor, true);
					//graphics.DrawRectangle(itemRect, window->Appearance->HighlightColor, true);
				}
				else if (isHovered)
				{
					graphics.DrawRectangle(itemRect, window->Appearance->ItemCollectionHightlightBackground, true);
				}
				auto iconSize = window->ToScale(window->Appearance->SmallIconSize);
				auto iconMargin = window->ToScale(3u);
				Point textPosition{ itemRect.X + 2, ((static_cast<int>(itemHeight - textItemHeight) >> 1) + 1) + static_cast<int>(i * itemHeight) };
				if (m_interactionData->m_drawImages)
				{
					textPosition.X += (int)(iconSize + iconMargin * 2u);

					auto& icon = m_interactionData->m_items[offsetIndex].m_icon;
					if (icon)
					{
						auto iconSourceSize = icon.GetSize();
						auto positionY = ((itemHeight - iconSize) >> 1) + static_cast<int>(i * itemHeight);
						icon.Paste(iconSourceSize.ToRectangle(), graphics, { 3, (int)positionY, iconSize , iconSize });
					}
				}

				graphics.DrawString(textPosition, items[offsetIndex].m_text, window->Appearance->Foreground);
				/*if (isSelected)
				{
					graphics.DrawRectangle(itemRect, window->Appearance->HighlightBorderColor, false);
				}*/
			}
		}

		graphics.DrawRectangle(m_control->GetAppearance().BoxBorderColor, false);
	}

	void FloatBoxReactor::MouseLeave(Graphics& graphics, const ArgMouse& args)
	{
		m_state.m_hoveredIndex = -1;

		Update(graphics);
		GUI::MarkAsUpdated(*m_control);
	}

	void FloatBoxReactor::MouseMove(Graphics& graphics, const ArgMouse& args)
	{
		if (IsInside(args.Position))
		{
			auto window = m_control->Handle();
			auto itemHeight = window->ToScale(window->Appearance->ComboBoxItemHeight);
			auto index = (args.Position.Y - 1) / itemHeight;
			
			m_state.m_hoveredIndex = m_state.m_offset + index;

			Update(graphics);
			GUI::MarkAsUpdated(*m_control);
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
			m_state.m_selectedIndex = m_state.m_hoveredIndex;
			m_interactionData->m_isSelected = true;
		}

		m_control->Dispose();
	}

	void FloatBoxReactor::MouseWheel(Graphics& graphics, const ArgWheel& args)
	{
		if (!m_scrollBar)
		{
			return;
		}

		int direction = args.WheelDelta > 0 ? -1 : 1;
		int newOffset = std::clamp(m_state.m_offset + direction, (int)m_scrollBar->GetMin(), (int)m_scrollBar->GetMax());

		if (newOffset != m_state.m_offset)
		{
			m_state.m_offset = newOffset;
			m_scrollBar->SetValue(m_state.m_offset);

			m_scrollBar->Handle()->Renderer.Update();
			GUI::MarkAsUpdated(m_scrollBar->Handle());

			Update(graphics);
			GUI::MarkAsUpdated(*m_control);
		}
	}

	void FloatBoxReactor::KeyPressed(Graphics& graphics, const ArgKeyboard& args)
	{
	}

	void FloatBoxReactor::SetState(Float::InteractionData& selection)
	{
		m_interactionData = &selection;
		m_state.m_selectedIndex = selection.m_selectedIndex;
		selection.m_isSelected = false;

		UpdateScrollBar();
	}

	bool FloatBoxReactor::MoveSelectedItem(int direction)
	{
		int newIndex = (std::max)(0, (std::min)(m_state.m_selectedIndex + direction, (int)(m_interactionData->m_items.size()) - 1));
		if (m_state.m_selectedIndex != newIndex && !m_interactionData->m_items.empty())
		{
			m_state.m_selectedIndex = newIndex;
			if (m_scrollBar)
			{
				auto visibleItemsCount = (std::min)(m_interactionData->m_items.size(), m_interactionData->m_maxItemsToDisplay);
				bool redrawScrollbar = false;
				if (newIndex - m_state.m_offset < 0)
				{
					--m_state.m_offset;
					m_scrollBar->SetValue(m_state.m_offset);

					redrawScrollbar = true;
				}
				else if (newIndex - m_state.m_offset >= visibleItemsCount)
				{
					++m_state.m_offset;
					m_scrollBar->SetValue(m_state.m_offset);

					redrawScrollbar = true;
				}

				if (redrawScrollbar)
				{
					m_scrollBar->Handle()->Renderer.Update();
					GUI::UpdateWindow(m_scrollBar->Handle());
				}
			}

			GUI::UpdateWindow(*m_control);
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
		{
			return;
		}

		auto window = m_control->Handle();
		auto scrollSize = window->ToScale(window->Appearance->ScrollBarSize);
		Rectangle rect{ static_cast<int>(window->Size.Width - scrollSize) - 1, 1, scrollSize, window->Size.Height - 2u };
		if (!m_scrollBar)
		{
			m_scrollBar = std::make_unique<ScrollBar>(window, false, rect);
			m_scrollBar->GetEvents().ValueChanged.Connect([this](const ArgScrollBar& args) 
			{
				m_state.m_offset = args.Value;
				//BT_CORE_DEBUG << "m_state.m_offset =" << m_state.m_offset <<std::endl;
				
				GUI::UpdateWindow(*m_control);
			});
		}

		auto delta = m_interactionData->m_items.size() - m_interactionData->m_maxItemsToDisplay;
		m_scrollBar->SetMinMax(0, (int)delta);
		if (m_state.m_selectedIndex >= 0)
		{
			auto blockId = (size_t)m_state.m_selectedIndex / m_interactionData->m_maxItemsToDisplay;
			int value = static_cast<int>(blockId * m_interactionData->m_maxItemsToDisplay);
			value = std::clamp(value, 0, (int)delta);

			m_scrollBar->SetValue(value);
			m_state.m_offset = value;
		}
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