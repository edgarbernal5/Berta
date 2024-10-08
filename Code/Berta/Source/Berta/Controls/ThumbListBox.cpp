/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#include "btpch.h"
#include "ThumbListBox.h"

#include "Berta/GUI/Interface.h"
#include "Berta/GUI/ControlAppearance.h"
#include "Berta/GUI/EnumTypes.h"

namespace Berta
{

	void ThumbListBoxReactor::Init(ControlBase& control)
	{
		m_control = &control;
		m_module.Appearance = reinterpret_cast<ThumbListBoxAppearance*>(control.Handle()->Appereance.get());

		m_module.m_window = control.Handle();
	}

	void ThumbListBoxReactor::Update(Graphics& graphics)
	{
		auto window = m_control->Handle();
		bool enabled = m_control->GetEnabled();
		auto backgroundRect = window->Size.ToRectangle();
		auto innerMargin = window->ToScale(3u);
		
		graphics.DrawRectangle(window->Size.ToRectangle(), window->Appereance->BoxBackground, true);
		backgroundRect.Width -= innerMargin * 2u;
		backgroundRect.Height -= innerMargin * 2u;

		auto thumbSize = window->ToScale(m_module.ThumbnailSize);
		auto maxCardMargin = window->ToScale(8u);
		uint32_t maxCardWidth = maxCardMargin * 2u + thumbSize + innerMargin;
		auto totalCardsInRow = backgroundRect.Width / maxCardWidth;
		if (totalCardsInRow == 0)
		{
			totalCardsInRow = 1;
		}
		auto marginRemainder = backgroundRect.Width % maxCardWidth;
		auto cardHeight = window->ToScale(m_module.Appearance->ThumbnailCardHeight);

		Point offset{ (int)innerMargin, (int)innerMargin - m_module.m_state.m_offset };
		Size cardSize{ thumbSize, thumbSize + cardHeight };
		auto cardMargin = marginRemainder / totalCardsInRow;
		auto cardMarginHalf = cardMargin >> 1;
		auto totalRows = (uint32_t)(m_module.Items.size()) / totalCardsInRow;
		if ((uint32_t)(m_module.Items.size()) % totalCardsInRow != 0)
		{
			++totalRows;
		}
		auto contentSize = static_cast<int>(totalRows * (cardSize.Height + innerMargin)) - static_cast<int>(innerMargin);
		if (contentSize > static_cast<int>(backgroundRect.Height))
		{
			if (!m_module.m_scrollBar)
			{
				auto scrollSize = window->ToScale(window->Appereance->ScrollBarSize);
				Rectangle rect{ static_cast<int>(window->Size.Width - scrollSize) - 1, 1, scrollSize, window->Size.Height - 2u };

				m_module.m_scrollBar = std::make_unique<ScrollBar>(window, false, rect);
				m_module.m_scrollBar->GetEvents().ValueChanged.Connect([this](const ArgScrollBar& args)
					{
						m_module.m_state.m_offset = args.Value;

						m_control->Handle()->Renderer.Update();
						GUI::RefreshWindow(m_control->Handle());
					});
				backgroundRect.Width -= rect.Width;
			}
			m_module.m_scrollBar->SetMinMax(0, contentSize - static_cast<int>(backgroundRect.Height));
			m_module.m_scrollBar->SetPageStepValue(backgroundRect.Height);
			m_module.m_scrollBar->SetStepValue(cardSize.Height);
		}
		else if (m_module.m_scrollBar)
		{
			m_module.m_state.m_offset = 0;
			m_module.m_scrollBar.reset();
		}

		for (size_t i = 0; i < m_module.Items.size(); i++)
		{
			auto& item = m_module.Items[i];

			Rectangle cardRect{ offset.X + (int)cardMarginHalf, offset.Y, cardSize.Width, cardSize.Height };
			if (cardRect.Y >= 0 || cardRect.Y + cardRect.Height <= backgroundRect.Height)
			{
				bool isSelected = item.IsSelected;
				//graphics.DrawRectangle(cardRect, isSelected ? window->Appereance->HighlightColor : window->Appereance->Background, true);
				graphics.DrawRectangle(cardRect, window->Appereance->Background, true);

				auto lineColor = enabled ? (isSelected ? window->Appereance->BoxBorderHighlightColor : window->Appereance->BoxBorderColor) : window->Appereance->BoxBorderDisabledColor;
				graphics.DrawRectangle(cardRect, lineColor, false);
				graphics.DrawLine({ cardRect.X, cardRect.Y + (int)thumbSize }, { cardRect.X + (int)cardSize.Width, cardRect.Y + (int)thumbSize }, lineColor);

				Size imageSize = window->ToScale(item.Thumbnail.GetSize());
				Size thumbFrameSize{ thumbSize, thumbSize };
				auto center = thumbFrameSize - imageSize;
				center *= 0.5f;

				Rectangle thumbnailRect{ cardRect.X + (int)center.Width, cardRect.Y + (int)center.Height, imageSize.Width, imageSize.Height };
				item.Thumbnail.Paste(graphics, thumbnailRect);

				{
					Size cardTextSize{ thumbSize, cardHeight };
					auto center = cardTextSize - graphics.GetTextExtent(item.Text);
					center *= 0.5f;

					if (isSelected)
					{
						graphics.DrawRectangle({ cardRect.X , cardRect.Y + (int)thumbSize ,cardRect.Width, cardHeight }, window->Appereance->HighlightColor, true);
					}
					graphics.DrawString({ cardRect.X + (int)center.Width, cardRect.Y + (int)thumbSize + (int)center.Height }, item.Text, isSelected ? window->Appereance->HighlightTextColor : window->Appereance->Foreground);
				}
			}

			offset.X += cardMargin + cardSize.Width + innerMargin;
			if (offset.X + cardMargin + cardSize.Width >= (int)window->Size.Width)
			{
				offset.X = innerMargin;
				offset.Y += cardSize.Height + innerMargin;
			}
		}

		if (m_module.m_selection.m_started && m_module.m_selection.m_startPosition != m_module.m_selection.m_endPosition)
		{
			Point startPoint{ 
				(std::min)(m_module.m_selection.m_startPosition.X, m_module.m_selection.m_endPosition.X),
				(std::min)(m_module.m_selection.m_startPosition.Y, m_module.m_selection.m_endPosition.Y) 
			};
			
			Point endPoint{ 
				(std::max)(m_module.m_selection.m_startPosition.X, m_module.m_selection.m_endPosition.X),
				(std::max)(m_module.m_selection.m_startPosition.Y, m_module.m_selection.m_endPosition.Y) 
			};

			Size boxSize{ (uint32_t)(endPoint.X - startPoint.X), (uint32_t)(endPoint.Y - startPoint.Y) };
			Graphics selectionBox(boxSize);
			Color blendColor = window->Appereance->HighlightColor;
			selectionBox.DrawRectangle(blendColor, true);

			Rectangle blendRect{ startPoint.X, startPoint.Y + m_module.m_state.m_offset, boxSize.Width, boxSize.Height};
			graphics.Blend(blendRect, selectionBox, { 0,0 }, 0.5f);
		}

		graphics.DrawRectangle(window->Size.ToRectangle(), enabled ? window->Appereance->BoxBorderColor : window->Appereance->BoxBorderDisabledColor, false);
	}

	void ThumbListBoxReactor::Resize(Graphics& graphics, const ArgResize& args)
	{
		m_module.BuildGridCards();
		m_module.BuildItems();
		m_module.UpdateScrollBar();
		if (m_module.m_scrollBar)
		{
			auto scrollSize = m_module.m_window->ToScale(m_module.m_window->Appereance->ScrollBarSize);
			Rectangle rect{ static_cast<int>(m_module.m_window->Size.Width - scrollSize) - 1, 1, scrollSize, m_module.m_window->Size.Height - 2u };
			GUI::MoveWindow(m_module.m_scrollBar->Handle(), rect);
		}
	}

	void ThumbListBoxReactor::MouseDown(Graphics& graphics, const ArgMouse& args)
	{
		m_module.m_mouseDownPosition = args.Position;
		auto itemAtPosition = m_module.GetItemIndexAtMousePosition(args.Position);
		bool hitOnBlank = itemAtPosition == -1;

		bool hasChanged = false;
		auto savedLastSelectedIndex = m_module.m_selection.m_pressedIndex;
		m_module.m_selection.m_pressedIndex = itemAtPosition;
		if (!m_module.m_multiselection && hitOnBlank)
		{
			if (m_module.m_selection.m_selectedIndex != -1)
			{
				m_module.Items[m_module.m_selection.m_selectedIndex].IsSelected = false;
				m_module.m_selection.m_indexes.clear();
				hasChanged = true;
			}
		}
		else if (!m_module.m_multiselection && !hitOnBlank)
		{
			hasChanged = (savedLastSelectedIndex != itemAtPosition);
			if (hasChanged)
			{
				if (m_module.m_selection.m_selectedIndex != -1)
				{
					m_module.Items[m_module.m_selection.m_selectedIndex].IsSelected = false;
					m_module.m_selection.m_indexes.clear();
				}
				m_module.Items[itemAtPosition].IsSelected = true;
				m_module.m_selection.m_indexes.push_back(itemAtPosition);
				m_module.m_selection.m_selectedIndex = itemAtPosition;
			}
		}
		else if (m_module.m_multiselection && hitOnBlank)
		{
			auto logicalPosition = args.Position;
			logicalPosition.Y -= m_module.m_state.m_offset;
			m_module.m_selection.m_started = true;
			m_module.m_selection.m_startPosition = logicalPosition;
			m_module.m_selection.m_endPosition = logicalPosition;

			if (!m_module.m_ctrlPressed && !m_module.m_shiftPressed)
			{
				hasChanged = !m_module.m_selection.m_indexes.empty();
				for (auto& index : m_module.m_selection.m_indexes)
				{
					m_module.Items[index].IsSelected = false;
				}
				m_module.m_selection.m_indexes.clear();
			}

			//setcapture
			GUI::Capture(m_module.m_window);
		}

		if (hasChanged)
		{
			Update(graphics);
			GUI::UpdateDeferred(*m_control);
		}
	}

	void ThumbListBoxReactor::MouseMove(Graphics& graphics, const ArgMouse& args)
	{
		bool hasChanged = false;

		if (m_module.m_selection.m_started)
		{
			auto logicalPosition = args.Position;
			logicalPosition.Y -= m_module.m_state.m_offset;
			m_module.m_selection.m_endPosition = logicalPosition;

			if (m_module.m_ctrlPressed)
			{

			}

			hasChanged |= m_module.m_selection.m_endPosition != m_module.m_selection.m_startPosition;
		}

		if (hasChanged)
		{
			Update(graphics);
			GUI::UpdateDeferred(*m_control);
		}
	}

	void ThumbListBoxReactor::MouseUp(Graphics& graphics, const ArgMouse& args)
	{
		bool hasChanged = false;
		if (m_module.m_selection.m_started)
		{
			m_module.m_selection.m_started = false;
			hasChanged = true;
			GUI::ReleaseCapture(m_module.m_window);
		}

		if (hasChanged)
		{
			Update(graphics);
			GUI::UpdateDeferred(*m_control);
		}
	}

	void ThumbListBoxReactor::MouseWheel(Graphics& graphics, const ArgWheel& args)
	{
		if (!m_module.m_scrollBar)
		{
			return;
		}

		int direction = args.WheelDelta > 0 ? -1 : 1;
		direction *= m_module.m_scrollBar->GetStepValue();
		int newOffset = std::clamp(m_module.m_state.m_offset + direction, (int)m_module.m_scrollBar->GetMin(), (int)m_module.m_scrollBar->GetMax());

		if (newOffset != m_module.m_state.m_offset)
		{
			m_module.m_state.m_offset = newOffset;
			m_module.m_scrollBar->SetValue(m_module.m_state.m_offset);

			m_module.m_scrollBar->Handle()->Renderer.Update();
			GUI::UpdateDeferred(m_module.m_scrollBar->Handle());

			Update(graphics);
			GUI::UpdateDeferred(*m_control);
		}
	}

	void ThumbListBoxReactor::KeyPressed(Graphics& graphics, const ArgKeyboard& args)
	{
		m_module.m_shiftPressed = m_module.m_shiftPressed || args.Key == KeyboardKey::Shift;
		m_module.m_ctrlPressed = m_module.m_ctrlPressed || args.Key == KeyboardKey::Control;
	}

	void ThumbListBoxReactor::KeyReleased(Graphics& graphics, const ArgKeyboard& args)
	{
		if (args.Key == KeyboardKey::Shift) m_module.m_shiftPressed = false;
		if (args.Key == KeyboardKey::Control) m_module.m_ctrlPressed = false;
	}

	void ThumbListBoxReactor::Module::AddItem(const std::wstring& text, const Image& thumbnail)
	{
		auto& newItem = Items.emplace_back();
		newItem.Text = text;
		newItem.Thumbnail = thumbnail;

		BuildItems();
		BuildGridCards();
	}

	void ThumbListBoxReactor::Module::BuildGridCards()
	{
		/*if (Items.empty())
		{
			return;
		}

		auto backgroundRect = m_window->Size.ToRectangle();
		auto innerMargin = m_window->ToScale(3u);

		backgroundRect.Width -= innerMargin * 2u;
		backgroundRect.Height -= innerMargin * 2u;

		auto thumbSize = m_window->ToScale(ThumbnailSize);
		auto maxCardMargin = m_window->ToScale(8u);
		uint32_t maxCardWidth = maxCardMargin * 2u + thumbSize + innerMargin;
		auto totalCardsInRow = backgroundRect.Width / maxCardWidth;
		if (totalCardsInRow == 0)
		{
			totalCardsInRow = 1;
		}
		auto marginRemainder = backgroundRect.Width % maxCardWidth;
		auto cardHeight = m_window->ToScale(Appearance->ThumbnailCardHeight);

		Point offset{ (int)innerMargin, (int)innerMargin };
		Size cardSize{ thumbSize, thumbSize + cardHeight };
		auto cardMargin = marginRemainder / totalCardsInRow;
		auto cardMarginHalf = cardMargin >> 1;
		auto totalRowsInHeight = backgroundRect.Height / (cardSize.Height + innerMargin);
		if (backgroundRect.Height % (cardSize.Height + innerMargin) != 0)
		{
			++totalRowsInHeight;
		}

		m_gridCards.clear();
		size_t totalCards = totalRowsInHeight * totalCardsInRow;
		for (size_t i = 0; i < totalCards; i++)
		{
			auto& newCard = m_gridCards.emplace_back();
			newCard.PosSize = { offset.X + (int)cardMarginHalf, offset.Y, cardSize.Width, cardSize.Height };

			offset.X += cardMargin + cardSize.Width + innerMargin;
			if (offset.X + cardMargin + cardSize.Width >= (int)m_window->Size.Width)
			{
				offset.X = innerMargin;
				offset.Y += cardSize.Height + innerMargin;
			}
		}*/
	}

	void ThumbListBoxReactor::Module::Clear()
	{
	}

	void ThumbListBoxReactor::Module::SetThumbnailSize(uint32_t size)
	{
		if (ThumbnailSize == size)
		{
			return;
		}

		ThumbnailSize = size;

		BuildGridCards();
		BuildItems();
		UpdateScrollBar();

		m_window->Renderer.Update();
		GUI::RefreshWindow(m_window);
	}

	void ThumbListBoxReactor::Module::UpdateScrollBar()
	{
		auto backgroundRect = m_window->Size.ToRectangle();
		auto innerMargin = m_window->ToScale(3u);

		backgroundRect.Width -= innerMargin * 2u;
		backgroundRect.Height -= innerMargin * 2u;

		auto thumbSize = m_window->ToScale(ThumbnailSize);
		auto maxCardMargin = m_window->ToScale(8u);
		uint32_t maxCardWidth = maxCardMargin * 2u + thumbSize + innerMargin;
		auto totalCardsInRow = backgroundRect.Width / maxCardWidth;
		if (totalCardsInRow == 0)
		{
			totalCardsInRow = 1;
		}
		auto marginRemainder = backgroundRect.Width % maxCardWidth;
		auto cardHeight = m_window->ToScale(Appearance->ThumbnailCardHeight);

		Point offset{ (int)innerMargin, (int)innerMargin - m_state.m_offset };
		Size cardSize{ thumbSize, thumbSize + cardHeight };
		auto cardMargin = marginRemainder / totalCardsInRow;
		auto cardMarginHalf = cardMargin >> 1;
		auto totalRows = (uint32_t)(Items.size()) / totalCardsInRow;
		if ((uint32_t)(Items.size()) % totalCardsInRow != 0)
		{
			++totalRows;
		}

		bool needScrollBar = NeedsScrollBar();
		if (!needScrollBar && m_scrollBar)
		{
			m_scrollBar.reset();
			m_state.m_offset = 0;
			return;
		}

		auto contentSize = totalRows * (cardSize.Height + innerMargin) - innerMargin;
		if (!m_scrollBar)
		{
			auto scrollSize = m_window->ToScale(m_window->Appereance->ScrollBarSize);
			Rectangle rect{ static_cast<int>(m_window->Size.Width - scrollSize) - 1, 1, scrollSize, m_window->Size.Height - 2u };

			m_scrollBar = std::make_unique<ScrollBar>(m_window, false, rect);
			m_scrollBar->GetEvents().ValueChanged.Connect([this](const ArgScrollBar& args)
				{
					//BT_CORE_TRACE << " - thumb scroll value = " << args.Value << ". max = " << m_scrollBar->GetMax() << ". page step = " << m_scrollBar->GetPageStepValue() << std::endl;
					m_state.m_offset = args.Value;

					m_window->Renderer.Update();
					GUI::RefreshWindow(m_window);
				});
		}
		m_scrollBar->SetMinMax(0, (int)(contentSize - backgroundRect.Height));
		m_scrollBar->SetPageStepValue(backgroundRect.Height);
		m_scrollBar->SetStepValue(cardSize.Height);
	}

	void ThumbListBoxReactor::Module::OnMouseDown(const ArgMouse& args)
	{
		//m_mouseDownPosition = args.Position;
	}

	void ThumbListBoxReactor::Module::EnableMultiselection(bool enabled)
	{
		m_multiselection = enabled;
	}

	int ThumbListBoxReactor::Module::GetItemIndexAtMousePosition(const Point& position)
	{
		auto offsetPosition = position;
		offsetPosition.Y += m_state.m_offset;
		for (size_t i = 0; i < Items.size(); i++)
		{
			if (Items[i].PosSize.IsInside(offsetPosition))
			{
				return static_cast<int>(i);
			}
		}
		return -1;
	}

	std::vector<size_t> ThumbListBoxReactor::Module::GetSelectedItems() const
	{
		return m_selection.m_indexes;
	}

	void ThumbListBoxReactor::Module::BuildItems()
	{
		auto backgroundRect = m_window->Size.ToRectangle();
		auto innerMargin = m_window->ToScale(3u);

		backgroundRect.Width -= innerMargin * 2u;
		backgroundRect.Height -= innerMargin * 2u;
		
		auto thumbSize = m_window->ToScale(ThumbnailSize);
		auto maxCardMargin = m_window->ToScale(8u);
		uint32_t maxCardWidth = maxCardMargin * 2u + thumbSize + innerMargin;
		auto totalCardsInRow = backgroundRect.Width / maxCardWidth;
		if (totalCardsInRow == 0)
		{
			totalCardsInRow = 1;
		}
		auto marginRemainder = backgroundRect.Width % maxCardWidth;
		auto cardHeight = m_window->ToScale(Appearance->ThumbnailCardHeight);

		Point offset{ (int)innerMargin, (int)innerMargin };
		Size cardSize{ thumbSize, thumbSize + cardHeight };
		auto cardMargin = marginRemainder / totalCardsInRow;
		auto cardMarginHalf = cardMargin >> 1;
		auto totalRows = (uint32_t)(Items.size()) / totalCardsInRow;
		if ((uint32_t)(Items.size()) % totalCardsInRow != 0)
		{
			++totalRows;
		}

		for (size_t i = 0; i < Items.size(); i++)
		{
			auto& item = Items[i];

			item.PosSize = { offset.X + (int)cardMarginHalf, offset.Y, cardSize.Width, cardSize.Height };

			offset.X += cardMargin + cardSize.Width + innerMargin;
			if (offset.X + cardMargin + cardSize.Width >= (int)m_window->Size.Width)
			{
				offset.X = innerMargin;
				offset.Y += cardSize.Height + innerMargin;
			}
		}
	}

	bool ThumbListBoxReactor::Module::NeedsScrollBar() const
	{
		auto backgroundRect = m_window->Size.ToRectangle();
		auto innerMargin = m_window->ToScale(3u);

		backgroundRect.Width -= innerMargin * 2u;
		backgroundRect.Height -= innerMargin * 2u;

		auto thumbSize = m_window->ToScale(ThumbnailSize);
		auto maxCardMargin = m_window->ToScale(8u);
		uint32_t maxCardWidth = maxCardMargin * 2u + thumbSize + innerMargin;
		auto totalCardsInRow = backgroundRect.Width / maxCardWidth;
		if (totalCardsInRow == 0)
		{
			totalCardsInRow = 1;
		}
		auto marginRemainder = backgroundRect.Width % maxCardWidth;
		auto cardHeight = m_window->ToScale(Appearance->ThumbnailCardHeight);

		Point offset{ (int)innerMargin, (int)innerMargin - m_state.m_offset };
		Size cardSize{ thumbSize, thumbSize + cardHeight };
		auto cardMargin = marginRemainder / totalCardsInRow;
		auto totalRows = (uint32_t)(Items.size()) / totalCardsInRow;
		if ((uint32_t)(Items.size()) % totalCardsInRow != 0)
		{
			++totalRows;
		}
		auto contentSize = static_cast<int>(totalRows * (cardSize.Height + innerMargin)) - static_cast<int>(innerMargin);
		return contentSize > static_cast<int>(backgroundRect.Height);
	}

	ThumbListBox::ThumbListBox(Window* parent, const Rectangle& rectangle)
	{
		Create(parent, true, rectangle);
	}

	void ThumbListBox::AddItem(const std::wstring& text, const Image& thumbnail)
	{
		m_reactor.GetModule().AddItem(text, thumbnail);
	}

	void ThumbListBox::Clear()
	{
		m_reactor.GetModule().Clear();
	}

	void ThumbListBox::SetThumbnailSize(uint32_t size)
	{
		m_reactor.GetModule().SetThumbnailSize(size);
	}

	void ThumbListBox::EnableMultiselection(bool enabled)
	{
		m_reactor.GetModule().EnableMultiselection(enabled);
	}

	std::vector<size_t> ThumbListBox::GetSelected() const
	{
		return m_reactor.GetModule().GetSelectedItems();
	}
}
