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

		Rectangle backgroundRect;
		uint32_t totalRows;
		uint32_t totalCardsInRow;
		uint32_t innerMargin;
		uint32_t cardMargin;
		uint32_t cardMarginHalf;
		int contentSize;
		Size cardSize;
		m_module.CalculateViewport(backgroundRect, totalRows, totalCardsInRow, cardSize, contentSize, innerMargin, cardMargin, cardMarginHalf);

		graphics.DrawRectangle(window->Size.ToRectangle(), window->Appereance->BoxBackground, true);

		if (contentSize > static_cast<int>(backgroundRect.Height))
		{
			if (!m_module.m_scrollBar)
			{
				auto scrollSize = window->ToScale(window->Appereance->ScrollBarSize);
				Rectangle scrollRect{ static_cast<int>(window->Size.Width - scrollSize) - 1, 1, scrollSize, window->Size.Height - 2u };

				m_module.m_scrollBar = std::make_unique<ScrollBar>(window, false, scrollRect);
				m_module.m_scrollBar->GetEvents().ValueChanged.Connect([this](const ArgScrollBar& args)
					{
						m_module.m_state.m_offset = args.Value;

						m_control->Handle()->Renderer.Update();
						GUI::RefreshWindow(m_control->Handle());
					});
			}
			m_module.m_scrollBar->SetMinMax(0, contentSize - static_cast<int>(backgroundRect.Height));
			m_module.m_scrollBar->SetPageStepValue(backgroundRect.Height);
			m_module.m_scrollBar->SetStepValue(cardSize.Height);

			//m_module.m_state.m_offset = m_module.m_scrollBar->GetValue();
		}
		else if (m_module.m_scrollBar)
		{
			m_module.m_state.m_offset = 0;
			m_module.m_scrollBar.reset();
		}
		Point offset{ backgroundRect.X + (int)innerMargin, backgroundRect.Y + (int)innerMargin - m_module.m_state.m_offset };

		auto thumbSize = m_module.m_window->ToScale(m_module.ThumbnailSize);
		auto cardHeight = m_module.m_window->ToScale(m_module.Appearance->ThumbnailCardHeight);
		for (size_t i = 0, k = 1; i < m_module.Items.size(); i++, ++k)
		{
			auto& item = m_module.Items[i];

			Rectangle cardRect{ offset.X + (int)cardMarginHalf, offset.Y, cardSize.Width, cardSize.Height };
			if (cardRect.Y <= static_cast<int>(backgroundRect.Height) && cardRect.Y + static_cast<int>(cardRect.Height) >= 0)
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

			offset.X += cardMargin + cardSize.Width + innerMargin * 2u;
			if (k == totalCardsInRow)
			{
				k = 0;
				offset.X = backgroundRect.X + (int)innerMargin;
				offset.Y += cardSize.Height + innerMargin * 2u;
			}
		}

		if (m_module.m_mouseSelection.m_started && m_module.m_mouseSelection.m_startPosition != m_module.m_mouseSelection.m_endPosition)
		{
			Point startPoint{ 
				(std::min)(m_module.m_mouseSelection.m_startPosition.X, m_module.m_mouseSelection.m_endPosition.X),
				(std::min)(m_module.m_mouseSelection.m_startPosition.Y, m_module.m_mouseSelection.m_endPosition.Y) 
			};
			
			Point endPoint{ 
				(std::max)(m_module.m_mouseSelection.m_startPosition.X, m_module.m_mouseSelection.m_endPosition.X),
				(std::max)(m_module.m_mouseSelection.m_startPosition.Y, m_module.m_mouseSelection.m_endPosition.Y) 
			};

			Size boxSize{ (uint32_t)(endPoint.X - startPoint.X), (uint32_t)(endPoint.Y - startPoint.Y) };
			Color blendColor = window->Appereance->HighlightColor;
			Graphics selectionBox(boxSize);
			selectionBox.DrawRectangle(blendColor, true);
			selectionBox.DrawRectangle(window->Appereance->BoxBorderColor, false);

			Rectangle blendRect{ startPoint.X, startPoint.Y + m_module.m_state.m_offset, boxSize.Width, boxSize.Height};
			graphics.Blend(blendRect, selectionBox, { 0,0 }, 0.5f);
		}

		graphics.DrawRectangle(window->Size.ToRectangle(), enabled ? window->Appereance->BoxBorderColor : window->Appereance->BoxBorderDisabledColor, false);
	}

	void ThumbListBoxReactor::Resize(Graphics& graphics, const ArgResize& args)
	{
		m_module.UpdateScrollBar();
		m_module.BuildGridCards();
		m_module.BuildItems();

		if (m_module.m_scrollBar)
		{
			auto scrollSize = m_module.m_window->ToScale(m_module.m_window->Appereance->ScrollBarSize);
			Rectangle scrollRect{ static_cast<int>(m_module.m_window->Size.Width - scrollSize) - 1, 1, scrollSize, m_module.m_window->Size.Height - 2u };
			GUI::MoveWindow(m_module.m_scrollBar->Handle(), scrollRect);
		}
	}

	void ThumbListBoxReactor::MouseDown(Graphics& graphics, const ArgMouse& args)
	{
		m_module.m_mouseDownPosition = args.Position;
		auto itemAtPosition = m_module.GetItemIndexAtMousePosition(args.Position);
		bool hitOnBlank = itemAtPosition == -1;

		bool hasChanged = false;
		auto savedLastSelectedIndex = m_module.m_mouseSelection.m_pressedIndex;
		m_module.m_mouseSelection.m_pressedIndex = itemAtPosition;
		if (!m_module.m_multiselection && hitOnBlank)
		{
			if (m_module.m_mouseSelection.m_selectedIndex != -1)
			{
				m_module.Items[m_module.m_mouseSelection.m_selectedIndex].IsSelected = false;
				m_module.m_mouseSelection.m_selections.clear();
				m_module.m_mouseSelection.m_selectedIndex = -1;
				hasChanged = true;
			}
		}
		else if (!m_module.m_multiselection && !hitOnBlank)
		{
			hasChanged = (savedLastSelectedIndex != itemAtPosition);
			m_module.m_mouseSelection.m_inverseSelection = (m_module.m_ctrlPressed && !m_module.m_shiftPressed);
			if (hasChanged)
			{
				if (m_module.m_mouseSelection.m_selectedIndex != -1)
				{
					m_module.Items[m_module.m_mouseSelection.m_selectedIndex].IsSelected = false;
					m_module.m_mouseSelection.m_selections.clear();
				}
				m_module.Items[itemAtPosition].IsSelected = true;
				m_module.m_mouseSelection.m_selections.push_back(itemAtPosition);
				m_module.m_mouseSelection.m_selectedIndex = itemAtPosition;
			}
		}
		else if (m_module.m_multiselection && hitOnBlank)
		{
			auto logicalPosition = args.Position;
			logicalPosition.Y -= m_module.m_state.m_offset;
			m_module.m_mouseSelection.m_started = true;
			m_module.m_mouseSelection.m_startPosition = logicalPosition;
			m_module.m_mouseSelection.m_endPosition = logicalPosition;

			m_module.m_mouseSelection.m_inverseSelection = (m_module.m_ctrlPressed && !m_module.m_shiftPressed);

			if (!m_module.m_ctrlPressed && !m_module.m_shiftPressed)
			{
				hasChanged = !m_module.m_mouseSelection.m_selections.empty();
				for (auto& index : m_module.m_mouseSelection.m_selections)
				{
					m_module.Items[index].IsSelected = false;
				}
				m_module.m_mouseSelection.m_selections.clear();
				m_module.m_mouseSelection.m_alreadySelected.clear();
			}
			else
			{
				m_module.m_mouseSelection.m_selections.clear();
				for (size_t i = 0; i < m_module.Items.size(); i++)
				{
					if (m_module.Items[i].IsSelected)
					{
						m_module.m_mouseSelection.m_selections.push_back(i);
					}
				}
				
				m_module.m_mouseSelection.m_alreadySelected = m_module.m_mouseSelection.m_selections;
			}

			GUI::Capture(m_module.m_window);
		}
		else
		{
			BT_CORE_TRACE << "ctrl " << m_module.m_ctrlPressed << ". shift " << m_module.m_shiftPressed << std::endl;
			if (!m_module.m_ctrlPressed && !m_module.m_shiftPressed && (!args.ButtonState.RightButton || !m_module.Items[itemAtPosition].IsSelected))
			{
				m_module.ClearSelection();
			}
			if (!m_module.m_ctrlPressed && !m_module.m_shiftPressed)
			{
				m_module.Items[itemAtPosition].IsSelected = true;
				m_module.m_mouseSelection.m_selections.push_back(itemAtPosition);
			}
			else
			{
				if (m_module.m_shiftPressed && savedLastSelectedIndex != -1)
				{
					int minIndex = (std::min)(savedLastSelectedIndex, itemAtPosition);
					int maxIndex = (std::max)(savedLastSelectedIndex, itemAtPosition);
					for (size_t i = minIndex; i <= maxIndex; i++)
					{
						m_module.Items[i].IsSelected = true;
						auto it = std::find(m_module.m_mouseSelection.m_selections.begin(), m_module.m_mouseSelection.m_selections.end(), i);
						if (it == m_module.m_mouseSelection.m_selections.end())
						{
							m_module.m_mouseSelection.m_selections.push_back(i);
						}
					}
				}
				else
				{
					m_module.Items[itemAtPosition].IsSelected = !m_module.Items[itemAtPosition].IsSelected;
				
					if (m_module.Items[itemAtPosition].IsSelected)
					{
						m_module.m_mouseSelection.m_selections.push_back(itemAtPosition);
					}
					else
					{
						auto it = std::find(m_module.m_mouseSelection.m_selections.begin(), m_module.m_mouseSelection.m_selections.end(), itemAtPosition);
						if (it != m_module.m_mouseSelection.m_selections.end())
						{
							m_module.m_mouseSelection.m_selections.erase(it);
						}
					}
				}
			}
			hasChanged = true;
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

		if (m_module.m_mouseSelection.m_started)
		{
			auto logicalPosition = args.Position;
			logicalPosition.Y -= m_module.m_state.m_offset;
			m_module.m_mouseSelection.m_endPosition = logicalPosition;

			Point startPoint{
				(std::min)(m_module.m_mouseSelection.m_startPosition.X, m_module.m_mouseSelection.m_endPosition.X),
				(std::min)(m_module.m_mouseSelection.m_startPosition.Y, m_module.m_mouseSelection.m_endPosition.Y)
			};

			Point endPoint{
				(std::max)(m_module.m_mouseSelection.m_startPosition.X, m_module.m_mouseSelection.m_endPosition.X),
				(std::max)(m_module.m_mouseSelection.m_startPosition.Y, m_module.m_mouseSelection.m_endPosition.Y)
			};

			Size boxSize{ (uint32_t)(endPoint.X - startPoint.X), (uint32_t)(endPoint.Y - startPoint.Y) };

			hasChanged |= (boxSize.Width > 0 && boxSize.Height > 0);
			if ((boxSize.Width > 0 && boxSize.Height > 0))
			{
				Rectangle selectionRect{ startPoint.X, startPoint.Y + m_module.m_state.m_offset * 2, boxSize.Width, boxSize.Height};
				for (size_t i = 0; i < m_module.Items.size(); i++)
				{
					auto& item = m_module.Items[i];
					bool intersection = item.Bounds.Intersect(selectionRect);
					bool alreadySelected = std::find(m_module.m_mouseSelection.m_alreadySelected.begin(), m_module.m_mouseSelection.m_alreadySelected.end(), i) != m_module.m_mouseSelection.m_alreadySelected.end();

					if (m_module.m_mouseSelection.m_inverseSelection)
					{
						if (intersection && !alreadySelected || !intersection && alreadySelected)
						{
							item.IsSelected = true;
						}
						else if (intersection && alreadySelected || !intersection && !alreadySelected)
						{
							item.IsSelected = false;
						}
					}
					else
					{
						item.IsSelected = intersection || alreadySelected;
					}
				}
			}
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

		if (m_module.m_mouseSelection.m_started)
		{
			hasChanged = true;
			m_module.m_mouseSelection.m_started = false;
			m_module.m_mouseSelection.m_selections.clear();
			for (size_t i = 0; i < m_module.Items.size(); i++)
			{
				if (m_module.Items[i].IsSelected)
				{
					m_module.m_mouseSelection.m_selections.push_back(i);
				}
			}
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

	void ThumbListBoxReactor::Module::CalculateViewport(Rectangle& backgroundRect, uint32_t& totalRows, uint32_t& totalCardsInRow, Size& cardSize, int& contentSize, uint32_t& innerMargin, uint32_t& cardMargin, uint32_t& cardMarginHalf) const
	{
		backgroundRect = m_window->Size.ToRectangle();
		innerMargin = m_window->ToScale(3u);

		backgroundRect.Width -= innerMargin * 2u;
		backgroundRect.Height -= innerMargin * 2u;
		backgroundRect.X = innerMargin;
		backgroundRect.Y = innerMargin;

		auto scrollSize = m_window->ToScale(m_window->Appereance->ScrollBarSize);
		
		auto thumbSize = m_window->ToScale(ThumbnailSize);
		auto cardHeight = m_window->ToScale(Appearance->ThumbnailCardHeight);
		cardSize = { thumbSize, thumbSize + cardHeight };
		
		uint32_t cardWidthWithMargin = cardSize.Width + innerMargin * 2u;
		totalCardsInRow = backgroundRect.Width / cardWidthWithMargin;
		if (totalCardsInRow == 0)
		{
			totalCardsInRow = 1;
		}
		totalRows = (uint32_t)(Items.size()) / totalCardsInRow;
		if ((uint32_t)(Items.size()) % totalCardsInRow != 0)
		{
			++totalRows;
		}
		contentSize = static_cast<int>(totalRows * (cardSize.Height + innerMargin * 2u));
		if (contentSize > static_cast<int>(backgroundRect.Height))
		{
			backgroundRect.Width -= scrollSize;
			totalCardsInRow = backgroundRect.Width / cardWidthWithMargin;
			if (totalCardsInRow == 0)
			{
				totalCardsInRow = 1;
			}
			
			totalRows = (uint32_t)(Items.size()) / totalCardsInRow;
			if ((uint32_t)(Items.size()) % totalCardsInRow != 0)
			{
				++totalRows;
			}
			contentSize = static_cast<int>(totalRows * (cardSize.Height + innerMargin * 2u));
		}
		auto marginRemainder = backgroundRect.Width % cardWidthWithMargin;

		cardMargin = marginRemainder / totalCardsInRow;
		cardMarginHalf = cardMargin >> 1;
	}

	void ThumbListBoxReactor::Module::BuildGridCards()
	{

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

		UpdateScrollBar();
		BuildGridCards();
		BuildItems();

		m_window->Renderer.Update();
		GUI::RefreshWindow(m_window);
	}

	void ThumbListBoxReactor::Module::UpdateScrollBar()
	{
		Rectangle backgroundRect;
		uint32_t totalRows;
		uint32_t totalCardsInRow;
		uint32_t innerMargin;
		uint32_t cardMargin;
		uint32_t cardMarginHalf;
		int contentSize;
		Size cardSize;
		CalculateViewport(backgroundRect, totalRows, totalCardsInRow, cardSize, contentSize, innerMargin, cardMargin, cardMarginHalf);

		bool needScrollBar = contentSize > static_cast<int>(backgroundRect.Height);
		if (!needScrollBar && m_scrollBar)
		{
			m_scrollBar.reset();
			m_state.m_offset = 0;
			return;
		}

		if (!m_scrollBar)
		{
			auto scrollSize = m_window->ToScale(m_window->Appereance->ScrollBarSize);
			Rectangle scrollRect{ static_cast<int>(m_window->Size.Width - scrollSize) - 1, 1, scrollSize, m_window->Size.Height - 2u };

			m_scrollBar = std::make_unique<ScrollBar>(m_window, false, scrollRect);
			m_scrollBar->GetEvents().ValueChanged.Connect([this](const ArgScrollBar& args)
				{
					m_state.m_offset = args.Value;

					m_window->Renderer.Update();
					GUI::RefreshWindow(m_window);
				});
		}
		m_scrollBar->SetMinMax(0, (int)(contentSize - backgroundRect.Height));
		m_scrollBar->SetPageStepValue(backgroundRect.Height);
		m_scrollBar->SetStepValue(cardSize.Height);

		m_state.m_offset = m_scrollBar->GetValue();
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
			if (Items[i].Bounds.IsInside(offsetPosition))
			{
				return static_cast<int>(i);
			}
		}
		return -1;
	}

	void ThumbListBoxReactor::Module::ClearSelection()
	{
		for (size_t i = 0; i < m_mouseSelection.m_selections.size(); i++)
		{
			Items[m_mouseSelection.m_selections[i]].IsSelected = false;
		}
		m_mouseSelection.m_selections.clear();
	}

	std::vector<size_t> ThumbListBoxReactor::Module::GetSelectedItems() const
	{
		return m_mouseSelection.m_selections;
	}

	void ThumbListBoxReactor::Module::BuildItems()
	{
		Rectangle backgroundRect;
		uint32_t totalRows;
		uint32_t totalCardsInRow;
		uint32_t innerMargin;
		uint32_t cardMargin;
		uint32_t cardMarginHalf;
		int contentSize;
		Size cardSize;
		CalculateViewport(backgroundRect, totalRows, totalCardsInRow, cardSize, contentSize, innerMargin, cardMargin, cardMarginHalf);

		Point offset{ backgroundRect.X + (int)innerMargin, backgroundRect.Y + (int)innerMargin/* - m_state.m_offset*/ };
		for (size_t i = 0, k = 1; i < Items.size(); i++, ++k)
		{
			auto& item = Items[i];

			item.Bounds = { offset.X + (int)cardMarginHalf, offset.Y, cardSize.Width, cardSize.Height };

			offset.X += cardMargin + cardSize.Width + innerMargin * 2u;
			if (k == totalCardsInRow)
			{
				k = 0;
				offset.X = backgroundRect.X + (int)innerMargin;
				offset.Y += cardSize.Height + innerMargin * 2u;
			}
		}
	}

	ThumbListBox::ThumbListBox(Window* parent, const Rectangle& rectangle)
	{
		Create(parent, true, rectangle);

#if BT_DEBUG
		m_handle->Name = "ThumbListBox";
#endif
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
