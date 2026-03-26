/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#include "btpch.h"
#include "FloatBox.h"
#include "Berta/GUI/EnumTypes.h"
#include "Berta/Controls/ComboBox.h"

namespace Berta
{
	FloatBoxReactor::~FloatBoxReactor()
	{
	}

	void FloatBoxReactor::Init(ControlBase& control, Graphics* graphics)
	{
		m_control = &control;
		m_floatBox = reinterpret_cast<FloatBox*>(&control);

		m_comboBoxAppearance = reinterpret_cast<ReactorCore::ComboBox::Appearance*>(m_floatBox->GetOwner()->Appearance.get());
	}

	void FloatBoxReactor::Update(Graphics& graphics)
	{
		auto window = m_control->Handle();
		auto clientRect = window->ClientSize.ToRectangle();
		if (m_scrollBar)
		{
			clientRect.Width -= m_scrollBar->Handle()->ClientSize.Width;
		}
		graphics.FillRectangle(clientRect, window->Appearance->BoxBackground);
		
		if (m_interactionData)
		{
			auto& items = m_interactionData->m_items;
			auto visibleItemsCount = (std::min)(m_interactionData->m_items.size(), m_interactionData->m_maxItemsToDisplay);
			auto textItemHeight = graphics.GetTextExtent().Height;

			auto itemHeight = window->ToScale(m_comboBoxAppearance->ComboBoxItemHeight);

			for (size_t i = 0; i < visibleItemsCount; i++)
			{
				size_t offsetIndex = static_cast<size_t>(m_state.m_offset) + i;
				bool isSelected = m_state.m_selectedIndex == offsetIndex;
				bool isHovered = m_state.m_hoveredIndex == offsetIndex;
				Rectangle itemRect{ 2, 1 + static_cast<int>(i * itemHeight), clientRect.Width - 4,itemHeight };
				if (isSelected)
				{
					graphics.DrawRoundRectBox(itemRect, window->Appearance->HighlightColor, window->Appearance->HighlightBorderColor, true);
					//graphics.DrawRectangle(itemRect, window->Appearance->HighlightColor, true);
				}
				else if (isHovered)
				{
					graphics.FillRectangle(itemRect, window->Appearance->ItemCollectionHightlightBackground);
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

		graphics.DrawRectangle(clientRect, m_floatBox->GetAppearance().BoxBorderColor);
	}

	void FloatBoxReactor::MouseLeave(Graphics& graphics, const ArgMouse& args)
	{
		m_state.m_hoveredIndex = -1;

		GUI::MarkAsNeedUpdate(*m_control);
	}

	void FloatBoxReactor::MouseMove(Graphics& graphics, const ArgMouse& args)
	{
		if (IsInside(args.Position))
		{
			auto window = m_control->Handle();
			auto itemHeight = window->ToScale(m_comboBoxAppearance->ComboBoxItemHeight);
			size_t index = (args.Position.Y - 1) / itemHeight;
			
			m_state.m_hoveredIndex = m_state.m_offset + index;

			GUI::MarkAsNeedUpdate(*m_control);
		} else {
			m_state.m_hoveredIndex = std::nullopt;
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

			GUI::MarkAsNeedUpdate(m_scrollBar->Handle());

			GUI::MarkAsNeedUpdate(*m_control);
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
		if (m_interactionData->m_items.empty()) return false;

		size_t itemCount = m_interactionData->m_items.size();
		size_t newIndex = 0;
		
		if (!m_state.m_selectedIndex)
		{
			newIndex = (direction > 0) ? 0 : itemCount - 1;
		}
		else
		{
			if (direction > 0)
			{
				newIndex = std::min<size_t>(*m_state.m_selectedIndex + 1, itemCount - 1);
			}
			else
			{
				newIndex = (*m_state.m_selectedIndex > 0) ? *m_state.m_selectedIndex - 1 : 0;
			}
		}
		if (m_state.m_selectedIndex != newIndex)
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
		return (point.X > 0 && point.X < static_cast<int>(m_control->Handle()->ClientSize.Width) - 1 &&
			point.Y > 0 && point.Y < static_cast<int>(m_control->Handle()->ClientSize.Height) - 2);
	}

	void FloatBoxReactor::UpdateScrollBar()
	{
		bool needScrollBar = m_interactionData->m_items.size() > m_interactionData->m_maxItemsToDisplay;
		if (!needScrollBar)
		{
			if (m_scrollBar)
			{
				m_scrollBar.reset();
				m_state.m_offset = 0;
			}
			return;
		}

		auto window = m_control->Handle();
		auto scrollSize = window->ToScale(window->Appearance->ScrollBarSize);
		Rectangle rect{ static_cast<int>(window->ClientSize.Width - scrollSize) - 1, 1, scrollSize, window->ClientSize.Height - 2u };
		//TODO:
		//DrawBatch drawBatch(window);
		
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
		m_scrollBar->SetMinMax(0, static_cast<int>(delta));
		
		if (m_state.m_selectedIndex.has_value())
		{
			// Calculate which 'page' or block the selected item is in
			size_t index = m_state.m_selectedIndex.value();
			auto blockId = index / m_interactionData->m_maxItemsToDisplay;
			int value = static_cast<int>(blockId * m_interactionData->m_maxItemsToDisplay);
        
			value = std::clamp(value, 0, static_cast<int>(delta));

			m_scrollBar->SetValue(value);
			m_state.m_offset = value;
		}
		else
		{
			// If nothing is selected, ensure we start at the top
			m_scrollBar->SetValue(0);
			m_state.m_offset = 0;
		}
	}

	FloatBox::FloatBox(Window* parent, const Rectangle& rectangle)
	{
		Create(parent, false, rectangle, { false, false, false, false, true, false }, false);
		GUI::MakeWindowActive(m_handle, false, this->GetParent());

#if BT_DEBUG
		SetDebugName("FloatBox");
#endif
	}

	bool FloatBox::OnKeyPressed(const ArgKeyboard& args)
	{
		bool redraw = false;
		//GetReactor().KeyPressed(m_handle->Renderer.GetGraphics(), args);
		return redraw;
	}

	bool FloatBox::MoveSelectedItem(int direction)
	{
		return GetReactor().MoveSelectedItem(direction);
	}
}