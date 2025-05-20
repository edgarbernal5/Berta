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
#include "Berta/Paint/DrawBatch.h"

namespace Berta
{
	void ThumbListBoxReactor::Init(ControlBase& control)
	{
		m_control = &control;
		m_module.m_appearance = reinterpret_cast<ThumbListBoxAppearance*>(control.Handle()->Appearance.get());
		m_module.m_events = reinterpret_cast<ThumbListBoxEvents*>(control.Handle()->Events.get());

		m_module.m_window = control.Handle();
		m_module.m_control = m_control;
		m_module.CalculateViewport(m_module.m_viewport);
	}

	void ThumbListBoxReactor::Update(Graphics& graphics)
	{
		//BT_CORE_TRACE << "  - ThumbListBoxReactor::Update " << std::endl;
		auto window = m_control->Handle();
		bool enabled = m_control->GetEnabled();

		graphics.DrawRectangle(window->ClientSize.ToRectangle(), window->Appearance->BoxBackground, true);

		Point offset{ 0, -m_module.m_state.m_offset };

		auto thumbSize = m_module.m_window->ToScale(m_module.m_thumbnailSize);
		auto cardHeight = m_module.m_window->ToScale(m_module.m_appearance->ThumbnailCardHeight);
		Size thumbFrameSize{ thumbSize, thumbSize };

		for (size_t i = m_module.m_viewport.m_startingVisibleIndex; i < m_module.m_viewport.m_endingVisibleIndex; i++)
		{
			auto& item = m_module.m_items[i];

			Rectangle cardRect{ item.m_bounds.X + offset.X, item.m_bounds.Y + offset.Y, m_module.m_viewport.m_cardSize.Width, m_module.m_viewport.m_cardSize.Height };
			
			Rectangle thumbnailRect = { cardRect.X, cardRect.Y, thumbFrameSize.Width, thumbFrameSize.Height };
			const bool& isSelected = item.m_isSelected;
			bool isLastSelected = (int)i == m_module.m_mouseSelection.m_selectedIndex;
			graphics.DrawRectangle(cardRect, window->Appearance->ButtonBackground, true);
			graphics.DrawRectangle(thumbnailRect, window->Appearance->Background, true);

			if (item.m_thumbnail)
			{
				Size imageSize = window->ToScale(item.m_thumbnail.GetSize());
				Rectangle thumbnailImageRect;
				if (imageSize.Width > thumbFrameSize.Width || imageSize.Height > thumbFrameSize.Height)
				{
					thumbnailImageRect = { cardRect.X, cardRect.Y, thumbFrameSize.Width, thumbFrameSize.Height };
				}
				else
				{
					Point center = thumbFrameSize;
					center -= imageSize;
					center /= 2;

					thumbnailImageRect = { cardRect.X + center.X, cardRect.Y + center.Y, imageSize.Width, imageSize.Height };
				}

				item.m_thumbnail.Paste(graphics, thumbnailImageRect);
			}
			
			
			if (isSelected)
			{
				graphics.DrawRectangle({ cardRect.X , cardRect.Y + (int)thumbSize, cardRect.Width, cardHeight }, window->Appearance->HighlightColor, true);
			}
			m_module.DrawItemText(graphics, item, { cardRect.X, cardRect.Y + (int)thumbSize, cardRect.Width, cardRect.Height });

			auto lineColor = enabled ? (isLastSelected ? window->Appearance->Foreground : (isSelected ? window->Appearance->BoxBorderHighlightColor : window->Appearance->BoxBorderColor)) : window->Appearance->BoxBorderDisabledColor;
			graphics.DrawRectangle(cardRect, lineColor, false);
			graphics.DrawLine({ cardRect.X, cardRect.Y + (int)thumbSize }, { cardRect.X + (int)m_module.m_viewport.m_cardSize.Width, cardRect.Y + (int)thumbSize }, lineColor);
		}

		if (m_module.m_mouseSelection.m_started && m_module.m_mouseSelection.m_startPosition != m_module.m_mouseSelection.m_endPosition)
		{
			Point startPoint, endPoint;
			Size boxSize;
			m_module.CalculateSelectionBox(startPoint, endPoint, boxSize);

			Color blendColor = m_module.m_window->Appearance->SelectionHighlightColor;
			Graphics selectionBox(boxSize, m_module.m_window->DPI, m_module.m_window->RootHandle);
			selectionBox.DrawRectangle(blendColor, true);
			selectionBox.DrawRectangle(m_module.m_window->Appearance->SelectionBorderHighlightColor, false);

			Rectangle blendRect{ startPoint.X, startPoint.Y + m_module.m_state.m_offset, boxSize.Width, boxSize.Height};
			graphics.Blend(blendRect, selectionBox, { 0,0 }, 0.5);
		}

		graphics.DrawRectangle(window->ClientSize.ToRectangle(), enabled ? window->Appearance->BoxBorderColor : window->Appearance->BoxBorderDisabledColor, false);
	}

	void ThumbListBoxReactor::Resize(Graphics& graphics, const ArgResize& args)
	{
		m_module.CalculateViewport(m_module.m_viewport);

		m_module.UpdateScrollBar();
		m_module.BuildItems();
		m_module.CalculateVisibleIndices();

		if (m_module.m_scrollBar)
		{
			auto scrollSize = m_module.m_window->ToScale(m_module.m_window->Appearance->ScrollBarSize);
			Rectangle scrollRect{ static_cast<int>(m_module.m_window->ClientSize.Width - scrollSize) - 1, 1, scrollSize, m_module.m_window->ClientSize.Height - 2u };
			GUI::MoveWindow(m_module.m_scrollBar->Handle(), scrollRect);
		}
	}

	void ThumbListBoxReactor::MouseDown(Graphics& graphics, const ArgMouse& args)
	{
		auto itemIndexAtPosition = m_module.GetItemIndexAtMousePosition(args.Position);
		bool hitOnBlank = itemIndexAtPosition == -1;

		bool needUpdate = false;
		if (m_module.m_multiselection)
		{
			if (hitOnBlank)
			{
				m_module.StartSelectionRectangle(args.Position);
				needUpdate = m_module.ClearSelectionIfNeeded();
			}
			else
			{
				needUpdate = m_module.HandleMultiSelection(itemIndexAtPosition, args);
			}
		}
		else
		{
			if (hitOnBlank)
			{
				needUpdate = m_module.ClearSingleSelection();
			}
			else
			{
				needUpdate = m_module.UpdateSingleSelection(itemIndexAtPosition);
			}
		}

		if (needUpdate)
		{
			Update(graphics);
			GUI::MarkAsUpdated(*m_control);
		}
	}

	void ThumbListBoxReactor::MouseMove(Graphics& graphics, const ArgMouse& args)
	{
		bool needUpdate = false;

		if (m_module.m_mouseSelection.m_started)
		{
			auto logicalPosition = args.Position;
			logicalPosition.Y -= m_module.m_state.m_offset;
			m_module.m_mouseSelection.m_endPosition = logicalPosition;

			Point startPoint, endPoint;
			Size boxSize;
			m_module.CalculateSelectionBox(startPoint, endPoint, boxSize);

			needUpdate |= (boxSize.Width > 0 && boxSize.Height > 0);
			if (boxSize.Width > 0 && boxSize.Height > 0)
			{
				Rectangle selectionRect{ startPoint.X, startPoint.Y + m_module.m_state.m_offset * 2, boxSize.Width, boxSize.Height};
				for (size_t i = m_module.m_viewport.m_startingVisibleIndex; i < m_module.m_viewport.m_endingVisibleIndex; i++)
				{
					auto& item = m_module.m_items[i];
					bool intersection = item.m_bounds.Intersect(selectionRect);
					bool alreadySelected = m_module.m_mouseSelection.IsAlreadySelected(i);

					if (m_module.m_mouseSelection.m_inverseSelection)
					{
						if (intersection && !alreadySelected || !intersection && alreadySelected)
						{
							item.m_isSelected = true;
						}
						else if (intersection && alreadySelected || !intersection && !alreadySelected)
						{
							item.m_isSelected = false;
						}
					}
					else
					{
						item.m_isSelected = intersection || alreadySelected;
					}
				}
			}
		}

		if (needUpdate)
		{
			Update(graphics);
			GUI::MarkAsUpdated(*m_control);
		}
	}

	void ThumbListBoxReactor::MouseUp(Graphics& graphics, const ArgMouse& args)
	{
		bool needUpdate = false;

		if (m_module.m_mouseSelection.m_started)
		{
			Point startPoint, endPoint;
			Size boxSize;
			m_module.CalculateSelectionBox(startPoint, endPoint, boxSize);
			needUpdate = (boxSize.Width > 0 && boxSize.Height > 0);

			m_module.m_mouseSelection.m_started = false;
			m_module.m_mouseSelection.m_selections.clear();
			for (size_t i = 0; i < m_module.m_items.size(); i++)
			{
				if (m_module.m_items[i].m_isSelected)
				{
					m_module.m_mouseSelection.m_selections.push_back(i);
				}
			}
			GUI::ReleaseCapture(m_module.m_window);
		}

		if (needUpdate)
		{
			Update(graphics);
			GUI::MarkAsUpdated(*m_control);
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
			m_module.CalculateVisibleIndices();
			m_module.m_scrollBar->SetValue(m_module.m_state.m_offset);

			m_module.m_scrollBar->Handle()->Renderer.Update();
			GUI::MarkAsUpdated(m_module.m_scrollBar->Handle());

			Update(graphics);
			GUI::MarkAsUpdated(*m_control);
		}
	}

	void ThumbListBoxReactor::KeyPressed(Graphics& graphics, const ArgKeyboard& args)
	{
		m_module.m_shiftPressed = m_module.m_shiftPressed || args.Key == KeyboardKey::Shift;
		m_module.m_ctrlPressed = m_module.m_ctrlPressed || args.Key == KeyboardKey::Control;

		bool needUpdate = false;
		if (args.Key == KeyboardKey::ArrowLeft || args.Key == KeyboardKey::ArrowRight || args.Key == KeyboardKey::ArrowUp || args.Key == KeyboardKey::ArrowDown)
		{
			if (args.Key == KeyboardKey::ArrowLeft || args.Key == KeyboardKey::ArrowRight)
			{
				auto direction = args.Key == KeyboardKey::ArrowLeft ? -1 : 1;
				auto pivot = (m_module.m_mouseSelection.m_selectedIndex == -1 ? (direction == -1 ? (int)m_module.m_items.size() : -1) : m_module.m_mouseSelection.m_selectedIndex);
				auto newItemIndex = pivot + direction;
				if (newItemIndex >= 0 && newItemIndex < static_cast<int>(m_module.m_items.size()))
				{
					if (!m_module.m_ctrlPressed)
					{
						m_module.ClearSelection();
					}

					if (m_module.m_multiselection && m_module.m_shiftPressed && m_module.m_mouseSelection.m_pivotIndex != -1)
					{
						int endIndex = newItemIndex;
						int startIndex = m_module.m_mouseSelection.m_pivotIndex;
						int minIndex = (std::min)(startIndex, endIndex);
						int maxIndex = (std::max)(startIndex, endIndex);

						for (int current = minIndex; current <= maxIndex; ++current)
						{
							m_module.m_items[current].m_isSelected = true;
							m_module.m_mouseSelection.m_selections.push_back(current);
						}
						m_module.m_mouseSelection.m_selectedIndex = newItemIndex;
					}
					else if (m_module.m_ctrlPressed)
					{
						m_module.m_mouseSelection.m_selectedIndex += direction;
					}
					else
					{
						m_module.m_items[newItemIndex].m_isSelected = true;
						m_module.m_mouseSelection.m_selections.push_back(newItemIndex);
						m_module.m_mouseSelection.m_selectedIndex = newItemIndex;
						m_module.m_mouseSelection.m_pivotIndex = newItemIndex;
					}
					m_module.EnsureVisibility(m_module.m_mouseSelection.m_selectedIndex);

					needUpdate = true;
				}
			}
			else if (args.Key == KeyboardKey::ArrowUp || args.Key == KeyboardKey::ArrowDown)
			{
				auto direction = args.Key == KeyboardKey::ArrowUp ? -1 : 1;
				auto pivot = (m_module.m_mouseSelection.m_selectedIndex == -1 ? (direction == -1 ? (int)m_module.m_items.size() : -1) : m_module.m_mouseSelection.m_selectedIndex);
				auto newItemIndex = pivot + direction * m_module.m_viewport.m_totalCardsInRow;
				if (newItemIndex >= 0 && newItemIndex < (int)m_module.m_items.size())
				{
					if (!m_module.m_ctrlPressed)
					{
						m_module.ClearSelection();
					}

					if (m_module.m_multiselection && m_module.m_shiftPressed && m_module.m_mouseSelection.m_pivotIndex != -1)
					{
						int endIndex = newItemIndex;
						int startIndex = m_module.m_mouseSelection.m_pivotIndex;
						int minIndex = (std::min)(startIndex, endIndex);
						int maxIndex = (std::max)(startIndex, endIndex);

						for (int current = minIndex; current <= maxIndex; ++current)
						{
							m_module.m_items[current].m_isSelected = true;
							m_module.m_mouseSelection.m_selections.push_back(current);
						}
						m_module.m_mouseSelection.m_selectedIndex = newItemIndex;
					}
					else if (m_module.m_ctrlPressed)
					{
						m_module.m_mouseSelection.m_selectedIndex = newItemIndex;
					}
					else
					{
						m_module.m_items[newItemIndex].m_isSelected = true;
						m_module.m_mouseSelection.m_selections.push_back(newItemIndex);
						m_module.m_mouseSelection.m_selectedIndex = newItemIndex;
						m_module.m_mouseSelection.m_pivotIndex = newItemIndex;
					}
					m_module.EnsureVisibility(m_module.m_mouseSelection.m_selectedIndex);
					needUpdate = true;
				}
			}
		}
		else if (args.Key == KeyboardKey::Space && m_module.m_ctrlPressed)
		{
			if (m_module.m_mouseSelection.m_selectedIndex != -1)
			{
				if (!m_module.m_multiselection && !m_module.m_mouseSelection.m_selections.empty())
				{
					m_module.m_items[m_module.m_mouseSelection.m_selections[0]].m_isSelected = false;
					m_module.m_mouseSelection.m_selections.clear();
				}

				auto& isSelected = m_module.m_items[m_module.m_mouseSelection.m_selectedIndex].m_isSelected;
				isSelected = !isSelected;
				if (isSelected)
				{
					m_module.m_mouseSelection.Select(m_module.m_mouseSelection.m_selectedIndex);
				}
				else
				{
					m_module.m_mouseSelection.Deselect(m_module.m_mouseSelection.m_selectedIndex);
				}

				if (m_module.m_multiselection)
				{
					m_module.m_mouseSelection.m_pivotIndex = m_module.m_mouseSelection.m_selectedIndex;
				}
				needUpdate = true;
			}
		}

		if (needUpdate)
		{
			Update(graphics);
			GUI::MarkAsUpdated(*m_control);
		}
	}

	void ThumbListBoxReactor::KeyReleased(Graphics& graphics, const ArgKeyboard& args)
	{
		if (args.Key == KeyboardKey::Shift) m_module.m_shiftPressed = false;
		if (args.Key == KeyboardKey::Control) m_module.m_ctrlPressed = false;
	}

	ThumbListBoxItem ThumbListBoxReactor::Module::At(size_t index)
	{
		return ThumbListBoxItem{ m_items[index], *this };
	}

	bool ThumbListBoxReactor::Module::AddItem(const std::wstring& text)
	{
		auto& newItem = m_items.emplace_back();
		newItem.m_text = text;

		CalculateViewport(m_viewport);

		UpdateScrollBar();
		BuildItems();

		auto savedEndingIndex = m_viewport.m_endingVisibleIndex;
		CalculateVisibleIndices();

		bool hasChanged = savedEndingIndex != m_viewport.m_endingVisibleIndex;
		if (hasChanged)
		{
			EmitVisibilityEvent(savedEndingIndex, true);
		}

		return hasChanged;
	}

	bool ThumbListBoxReactor::Module::AddItem(const std::wstring& text, const Image& thumbnail)
	{
		auto& newItem = m_items.emplace_back();
		newItem.m_text = text;
		newItem.m_thumbnail = thumbnail;

		CalculateViewport(m_viewport);

		UpdateScrollBar();
		BuildItems();

		auto savedEndingIndex = m_viewport.m_endingVisibleIndex;
		CalculateVisibleIndices();

		bool hasChanged = savedEndingIndex != m_viewport.m_endingVisibleIndex;
		if (hasChanged)
		{
			EmitVisibilityEvent(savedEndingIndex, true);
		}

		return hasChanged;
	}

	void ThumbListBoxReactor::Module::CalculateViewport(ViewportData& viewportData) const
	{
		viewportData.m_backgroundRect = m_window->ClientSize.ToRectangle();
		viewportData.m_innerMargin = m_window->ToScale(3u);

		viewportData.m_backgroundRect.Width -= viewportData.m_innerMargin * 2u;
		viewportData.m_backgroundRect.Height -= viewportData.m_innerMargin * 2u;
		viewportData.m_backgroundRect.X = viewportData.m_innerMargin;
		viewportData.m_backgroundRect.Y = viewportData.m_innerMargin;

		auto scrollSize = m_window->ToScale(m_window->Appearance->ScrollBarSize);
		
		auto thumbSize = m_window->ToScale(m_thumbnailSize);
		auto cardHeight = m_window->ToScale(m_appearance->ThumbnailCardHeight);
		viewportData.m_cardSize = { thumbSize, thumbSize + cardHeight };
		
		uint32_t cardWidthWithMargin = viewportData.m_cardSize.Width + viewportData.m_innerMargin * 2u;
		viewportData.m_totalCardsInRow = viewportData.m_backgroundRect.Width / cardWidthWithMargin;
		if (viewportData.m_totalCardsInRow == 0)
		{
			viewportData.m_totalCardsInRow = 1;
		}
		viewportData.m_totalRows = (uint32_t)(m_items.size()) / viewportData.m_totalCardsInRow;
		if ((uint32_t)(m_items.size()) % viewportData.m_totalCardsInRow != 0)
		{
			++viewportData.m_totalRows;
		}

		viewportData.m_cardSizeWithMargin = { cardWidthWithMargin, viewportData.m_cardSize.Height + viewportData.m_innerMargin * 2u };
		viewportData.m_contentSize = static_cast<int>(viewportData.m_totalRows * viewportData.m_cardSizeWithMargin.Height);
		if (viewportData.m_contentSize > static_cast<int>(viewportData.m_backgroundRect.Height))
		{
			viewportData.m_backgroundRect.Width -= scrollSize;
			viewportData.m_totalCardsInRow = viewportData.m_backgroundRect.Width / cardWidthWithMargin;
			if (viewportData.m_totalCardsInRow == 0)
			{
				viewportData.m_totalCardsInRow = 1;
			}
			
			viewportData.m_totalRows = (uint32_t)(m_items.size()) / viewportData.m_totalCardsInRow;
			if ((uint32_t)(m_items.size()) % viewportData.m_totalCardsInRow != 0)
			{
				++viewportData.m_totalRows;
			}
			viewportData.m_contentSize = static_cast<int>(viewportData.m_totalRows * viewportData.m_cardSizeWithMargin.Height);
		}
		auto marginRemainder = viewportData.m_backgroundRect.Width % cardWidthWithMargin;

		viewportData.m_cardMargin = marginRemainder / viewportData.m_totalCardsInRow;
		viewportData.m_cardMarginHalf = viewportData.m_cardMargin >> 1;
	}

	void ThumbListBoxReactor::Module::CalculateVisibleIndices()
	{
		if (m_items.empty() || !m_scrollBar)
		{
			m_viewport.m_startingVisibleIndex = 0;
			m_viewport.m_endingVisibleIndex = (int)m_items.size();
			return;
		}

		int viewportHeight = (int)(m_viewport.m_backgroundRect.Height);
		int cardSizeHeightWithMargin = (int)(m_viewport.m_cardSizeWithMargin.Height);
		int startRow = m_state.m_offset / cardSizeHeightWithMargin;
		int endRow = 1 + (m_state.m_offset + viewportHeight) / cardSizeHeightWithMargin;

		m_viewport.m_startingVisibleIndex = startRow * m_viewport.m_totalCardsInRow;
		m_viewport.m_endingVisibleIndex = (std::min)(endRow * (int)m_viewport.m_totalCardsInRow, (int)m_items.size());
	}

	void ThumbListBoxReactor::Module::CalculateSelectionBox(Point& startPoint, Point& endPoint, Size& boxSize)
	{
		startPoint = {
			(std::min)(m_mouseSelection.m_startPosition.X, m_mouseSelection.m_endPosition.X),
			(std::min)(m_mouseSelection.m_startPosition.Y, m_mouseSelection.m_endPosition.Y)
		};

		endPoint = {
			(std::max)(m_mouseSelection.m_startPosition.X, m_mouseSelection.m_endPosition.X),
			(std::max)(m_mouseSelection.m_startPosition.Y, m_mouseSelection.m_endPosition.Y)
		};

		boxSize = { (uint32_t)(endPoint.X - startPoint.X), (uint32_t)(endPoint.Y - startPoint.Y) };
	}

	bool ThumbListBoxReactor::Module::Clear()
	{
		bool needUpdate = !m_items.empty();
		m_items.clear();

		m_mouseSelection.Clear();
		CalculateViewport(m_viewport);
		CalculateVisibleIndices();

		if (needUpdate)
		{
			if (m_scrollBar)
			{
				m_scrollBar.reset();
				m_state.m_offset = 0;
			}
		}
		return needUpdate;
	}

	void ThumbListBoxReactor::Module::Erase(size_t index)
	{
		if (m_items.size() <= index)
		{
			return;
		}
		
		for (size_t i = 0; i < m_mouseSelection.m_selections.size(); i++)
		{
			auto& itemIndex = m_mouseSelection.m_selections[i];
			if (itemIndex > index)
				--itemIndex;
		}

		auto it = m_items.begin() + index;
		m_items.erase(it);

		CalculateViewport(m_viewport);
		UpdateScrollBar();
		BuildItems();
		CalculateVisibleIndices();

		if (m_scrollBar)
		{
			m_scrollBar->Handle()->Renderer.Update();
		}

		GUI::UpdateWindow(m_window);
	}

	void ThumbListBoxReactor::Module::SetThumbnailSize(uint32_t size)
	{
		if (m_thumbnailSize == size)
		{
			return;
		}

		m_thumbnailSize = size;

		CalculateViewport(m_viewport);
		UpdateScrollBar();
		BuildItems();
		CalculateVisibleIndices();

		if (m_scrollBar)
		{
			m_scrollBar->Handle()->Renderer.Update();
		}

		GUI::UpdateWindow(m_window);
	}

	void ThumbListBoxReactor::Module::UpdateScrollBar()
	{
		bool needScrollBar = m_viewport.m_contentSize > static_cast<int>(m_viewport.m_backgroundRect.Height);
		if (!needScrollBar)
		{
			m_scrollBar.reset();
			m_state.m_offset = 0;
			return;
		}

		if (!m_scrollBar)
		{
			auto scrollSize = m_window->ToScale(m_window->Appearance->ScrollBarSize);
			Rectangle scrollRect{ static_cast<int>(m_window->ClientSize.Width - scrollSize) - 1, 1, scrollSize, m_window->ClientSize.Height - 2u };

			m_scrollBar = std::make_unique<ScrollBar>(m_window, false, scrollRect);
			m_scrollBar->GetEvents().ValueChanged.Connect([this](const ArgScrollBar& args)
				{
					m_state.m_offset = args.Value;
					auto savedStartingIndex = m_viewport.m_startingVisibleIndex;
					auto savedEndingIndex = m_viewport.m_endingVisibleIndex;
					CalculateVisibleIndices();

					if (savedStartingIndex != m_viewport.m_startingVisibleIndex || savedEndingIndex != m_viewport.m_endingVisibleIndex)
					{
						for (size_t i = savedStartingIndex; i < m_viewport.m_startingVisibleIndex; i++)
						{
							EmitVisibilityEvent(i, false);
						}
						for (size_t i = m_viewport.m_startingVisibleIndex; i < savedStartingIndex; i++)
						{
							EmitVisibilityEvent(i, true);
						}

						for (size_t i = m_viewport.m_endingVisibleIndex; i < savedEndingIndex; i++)
						{
							EmitVisibilityEvent(i, false);
						}

						for (size_t i = savedEndingIndex; i < m_viewport.m_endingVisibleIndex; i++)
						{
							EmitVisibilityEvent(i, true);
						}
					}

					GUI::UpdateWindow(m_window);
				});
		}

		DrawBatch drawBatch(*m_scrollBar);
		m_scrollBar->SetMinMax(0, (int)(m_viewport.m_contentSize - m_viewport.m_backgroundRect.Height));
		m_scrollBar->SetPageStepValue(m_viewport.m_backgroundRect.Height);
		m_scrollBar->SetStepValue(m_viewport.m_cardSize.Height);

		m_state.m_offset = m_scrollBar->GetValue();
	}

	bool ThumbListBoxReactor::Module::IsEnabledMultiselection() const
	{
		return m_multiselection;
	}

	bool ThumbListBoxReactor::Module::EnableMultiselection(bool enabled)
	{
		bool needUpdate = m_multiselection != enabled;
		m_multiselection = enabled;
		return needUpdate;
	}

	int ThumbListBoxReactor::Module::GetItemIndexAtMousePosition(const Point& position)
	{
		auto offsetPosition = position;
		offsetPosition.Y += m_state.m_offset;

		for (size_t i = m_viewport.m_startingVisibleIndex; i < m_viewport.m_endingVisibleIndex; i++)
		{
			if (m_items[i].m_bounds.IsInside(offsetPosition))
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
			m_items[m_mouseSelection.m_selections[i]].m_isSelected = false;
		}
		m_mouseSelection.m_selections.clear();
	}

	bool ThumbListBoxReactor::Module::ClearSingleSelection()
	{
		if (m_mouseSelection.m_selectedIndex != -1)
		{
			m_items[m_mouseSelection.m_selectedIndex].m_isSelected = false;
			m_mouseSelection.m_selections.clear();
			m_mouseSelection.m_selectedIndex = -1;
			m_mouseSelection.m_pivotIndex = -1;
			return true;
		}
		return false;
	}

	bool ThumbListBoxReactor::Module::UpdateSingleSelection(int newItemIndex)
	{
		bool needUpdate = m_mouseSelection.m_selectedIndex != newItemIndex || !m_items[newItemIndex].m_isSelected;
		if (needUpdate)
		{
			ClearSingleSelection();
			SelectItem(newItemIndex);
			m_mouseSelection.m_pivotIndex = newItemIndex;
		}
		return needUpdate;
	}

	void ThumbListBoxReactor::Module::SelectItem(int index)
	{
		m_items[index].m_isSelected = true;
		m_mouseSelection.m_selections.push_back(index);
		m_mouseSelection.m_selectedIndex = index;
	}

	void ThumbListBoxReactor::Module::StartSelectionRectangle(const Point& mousePosition)
	{
		auto logicalPosition = mousePosition;
		logicalPosition.Y -= m_state.m_offset;
		m_mouseSelection.m_started = true;
		m_mouseSelection.m_startPosition = logicalPosition;
		m_mouseSelection.m_endPosition = logicalPosition;

		m_mouseSelection.m_inverseSelection = (m_ctrlPressed && !m_shiftPressed);

		m_mouseSelection.m_selections.clear();
		m_mouseSelection.m_alreadySelected.clear();

		for (size_t i = 0; i < m_items.size(); i++)
		{
			if (m_items[i].m_isSelected)
			{
				m_mouseSelection.m_selections.push_back(i);
				m_mouseSelection.m_alreadySelected.push_back(i);
			}
		}

		GUI::Capture(m_window);
	}

	bool ThumbListBoxReactor::Module::ClearSelectionIfNeeded()
	{
		if (!m_ctrlPressed && !m_shiftPressed)
		{
			if (!m_mouseSelection.m_selections.empty())
			{
				for (const auto& index : m_mouseSelection.m_selections)
				{
					m_items[index].m_isSelected = false;
				}
				m_mouseSelection.m_selections.clear();
				m_mouseSelection.m_alreadySelected.clear();

				//m_mouseSelection.m_selectedIndex = -1;
				m_mouseSelection.m_pivotIndex = -1;
				return true;
			}
		}
		return false;
	}

	void ThumbListBoxReactor::Module::ToggleItemSelection(int itemIndexAtPosition)
	{
		auto& item = m_items[itemIndexAtPosition];
		item.m_isSelected = !item.m_isSelected;

		if (item.m_isSelected)
		{
			m_mouseSelection.m_selections.push_back(itemIndexAtPosition);
		}
		else
		{
			auto it = std::remove(m_mouseSelection.m_selections.begin(), m_mouseSelection.m_selections.end(), itemIndexAtPosition);
			m_mouseSelection.m_selections.erase(it, m_mouseSelection.m_selections.end());
		}
	}

	void ThumbListBoxReactor::Module::PerformRangeSelection(int itemIndexAtPosition)
	{
		int minIndex = (std::min)(m_mouseSelection.m_pivotIndex, itemIndexAtPosition);
		int maxIndex = (std::max)(m_mouseSelection.m_pivotIndex, itemIndexAtPosition);

		for (int i = minIndex; i <= maxIndex; ++i)
		{
			m_items[i].m_isSelected = true;
			if (!m_mouseSelection.IsSelected(i))
			{
				m_mouseSelection.m_selections.push_back(i);
			}
		}
	}

	bool ThumbListBoxReactor::Module::HandleMultiSelection(int itemIndexAtPosition, const ArgMouse& args)
	{
		bool needUpdate = false;
		if (m_mouseSelection.m_pivotIndex == -1)
		{
			m_mouseSelection.m_pivotIndex = itemIndexAtPosition;
		}

		if (!m_ctrlPressed && !m_shiftPressed)
		{
			if (!args.ButtonState.RightButton || !m_items[itemIndexAtPosition].m_isSelected)
			{
				ClearSelection();
			}
			if (!m_items[itemIndexAtPosition].m_isSelected)
			{
				SelectItem(itemIndexAtPosition);
				needUpdate = true;
			}
		}
		else if (m_shiftPressed && m_mouseSelection.m_selectedIndex != -1)
		{
			PerformRangeSelection(itemIndexAtPosition);
			needUpdate = true;
		}
		else
		{
			ToggleItemSelection(itemIndexAtPosition);
			needUpdate = true;
		}
		needUpdate |= m_mouseSelection.m_selectedIndex != itemIndexAtPosition;
		m_mouseSelection.m_selectedIndex = itemIndexAtPosition;

		needUpdate |= EnsureVisibility(itemIndexAtPosition);
		return needUpdate;
	}

	void ThumbListBoxReactor::Module::EmitVisibilityEvent(size_t index, bool visible) const
	{
		ArgThumbListBoxItemVisibility args;
		args.Index = index;
		args.Visible = visible;
		BT_CORE_DEBUG << " - visibility item = " << index << ". visible=" << visible << std::endl;
		m_events->ItemVisibility.Emit(args);
	}

	void ThumbListBoxReactor::Module::Draw() const
	{
		GUI::UpdateWindow(m_window);
	}

	void ThumbListBoxReactor::Module::DrawItem(Graphics& graphics, ItemType& item, Point& offset)
	{
	}

	void ThumbListBoxReactor::Module::DrawItemText(Graphics& graphics, ItemType& item, const Rectangle& cardRect)
	{
		auto wholeExtent = graphics.GetTextExtent(item.m_text);

		if (wholeExtent.Width < cardRect.Width)
		{
			Point targetSub{ cardRect.X, cardRect.Y };
			targetSub.X += (int)(cardRect.Width - wholeExtent.Width) >> 1;
			graphics.DrawString(targetSub, item.m_text, m_appearance->Foreground);
			return;
		}

		auto words = StringUtils::Split(item.m_text, ' ');
		Point offset{};
		for (size_t i = 0; i < words.size(); i++)
		{
			auto& word = words[i];
			auto wordExtent = graphics.GetTextExtent(word);

			if (wordExtent.Width + offset.X >= cardRect.Width)
			{
				for (size_t j = word.size() - 1; j > 0; --j)
				{
					auto subExtent = graphics.GetTextExtent(word, j);
					if (subExtent.Width + offset.X < cardRect.Width)
					{
						Point targetSub{ offset.X + cardRect.X, offset.Y + cardRect.Y};
						targetSub.X += (int)(cardRect.Width - subExtent.Width) >> 1;
						graphics.DrawString(targetSub, word.substr(0, j), m_appearance->Foreground);

						offset.X += subExtent.Width;
						break;
					}
				}
			}
			else
			{
				Point targetSub{ offset.X + cardRect.X, offset.Y + cardRect.Y };
				targetSub.X += (int)(cardRect.Width - wordExtent.Width) >> 1;
				graphics.DrawString(targetSub, word, m_appearance->Foreground);
				offset.Y += wordExtent.Height;

				offset.X = 0;
			}
		}
	}

	std::vector<size_t> ThumbListBoxReactor::Module::GetSelectedItems() const
	{
		return m_mouseSelection.m_selections;
	}

	bool ThumbListBoxReactor::Module::EnsureVisibility(int lastSelectedIndex)
	{
		if (!m_scrollBar)
		{
			return false;
		}

		auto itemBounds = m_items[lastSelectedIndex].m_bounds;
		itemBounds.Y -= m_state.m_offset;

		if (m_viewport.m_backgroundRect.Contains(itemBounds))
		{
			return false;
		}

		int itemHeight = static_cast<int>(itemBounds.Height);
		int viewportHeight = static_cast<int>(m_viewport.m_backgroundRect.Height);
		int innerMarginDouble = static_cast<int>(m_viewport.m_innerMargin * 2u);
		auto offsetAdjustment = 0;
		if (itemBounds.Y >= viewportHeight)
		{
			offsetAdjustment = itemHeight + innerMarginDouble;
		}
		else if (itemBounds.Y + itemHeight <= 0)
		{
			offsetAdjustment = -itemHeight - innerMarginDouble;
		}
		else if (itemBounds.Y + itemHeight > 0 && itemBounds.Y + itemHeight < viewportHeight)
		{
			offsetAdjustment = (itemBounds.Y + itemHeight) - itemHeight - innerMarginDouble;
		}
		else
		{
			offsetAdjustment = (itemBounds.Y + itemHeight) - viewportHeight;
		}
		m_state.m_offset = std::clamp(m_state.m_offset + offsetAdjustment, m_scrollBar->GetMin(), m_scrollBar->GetMax());
		auto savedStartingIndex = m_viewport.m_startingVisibleIndex;
		auto savedEndingIndex = m_viewport.m_endingVisibleIndex;
		CalculateVisibleIndices();

		if (savedStartingIndex != m_viewport.m_startingVisibleIndex || savedEndingIndex != m_viewport.m_endingVisibleIndex)
		{
			for (size_t i = savedStartingIndex; i < m_viewport.m_startingVisibleIndex; i++)
			{
				EmitVisibilityEvent(i, false);
			}
			for (size_t i = m_viewport.m_startingVisibleIndex; i < savedStartingIndex; i++)
			{
				EmitVisibilityEvent(i, true);
			}

			for (size_t i = m_viewport.m_endingVisibleIndex; i < savedEndingIndex; i++)
			{
				EmitVisibilityEvent(i, false);
			}

			for (size_t i = savedEndingIndex; i < m_viewport.m_endingVisibleIndex; i++)
			{
				EmitVisibilityEvent(i, true);
			}
		}

		m_scrollBar->SetValue(m_state.m_offset);

		GUI::UpdateWindow(m_scrollBar->Handle());
		return true;
	}

	void ThumbListBoxReactor::Module::UpdatedThumbnail(ItemType& item)
	{
		auto itemBounds = item.m_bounds;
		itemBounds.Y -= m_state.m_offset;

		if (!m_viewport.m_backgroundRect.Intersect(itemBounds))
		{
			return;
		}

		GUI::UpdateWindow(m_window);
	}

	void ThumbListBoxReactor::Module::BuildItems()
	{
		int innerMarginInt = (int)m_viewport.m_innerMargin;
		Point offset{ m_viewport.m_backgroundRect.X, m_viewport.m_backgroundRect.Y };
		for (size_t i = 0, k = 1; i < m_items.size(); i++, ++k)
		{
			auto& item = m_items[i];

			item.m_bounds = { offset.X + (int)m_viewport.m_cardMarginHalf + innerMarginInt, offset.Y + innerMarginInt, m_viewport.m_cardSize.Width, m_viewport.m_cardSize.Height };

			offset.X += m_viewport.m_cardMargin + m_viewport.m_cardSize.Width + m_viewport.m_innerMargin * 2u;
			if (k == m_viewport.m_totalCardsInRow)
			{
				k = 0;
				offset.X = m_viewport.m_backgroundRect.X;
				offset.Y += (int)(m_viewport.m_cardSize.Height + m_viewport.m_innerMargin * 2u);
			}
		}
	}

	bool ThumbListBoxReactor::Module::MouseSelection::IsAlreadySelected(size_t index) const
	{
		return std::find(m_alreadySelected.begin(), m_alreadySelected.end(), index) != m_alreadySelected.end();
	}

	bool ThumbListBoxReactor::Module::MouseSelection::IsSelected(size_t index) const
	{
		return std::find(m_selections.begin(), m_selections.end(), index) != m_selections.end();
	}

	void ThumbListBoxReactor::Module::MouseSelection::Select(size_t index)
	{
		m_selections.push_back(index);
	}

	void ThumbListBoxReactor::Module::MouseSelection::Deselect(size_t index)
	{
		auto it = std::find(m_selections.begin(), m_selections.end(), index);
		if (it != m_selections.end())
		{
			m_selections.erase(it);
		}
	}

	void ThumbListBoxReactor::Module::MouseSelection::Clear()
	{
		m_selectedIndex = -1;
		m_pivotIndex = -1;
		m_selections.clear();
		m_alreadySelected.clear();
	}

	void ThumbListBoxItem::SetIcon(const Image& image)
	{
		if (m_target.m_thumbnail == image)
			return;

		m_target.m_thumbnail = image;
		m_module.UpdatedThumbnail(m_target);
	}

	ThumbListBox::ThumbListBox(Window* parent, const Rectangle& rectangle)
	{
		Create(parent, true, rectangle);

#if BT_DEBUG
		m_handle->Name = "ThumbListBox";
#endif
	}

	void ThumbListBox::AddItem(const std::wstring& text)
	{
		if (m_reactor.GetModule().AddItem(text) && IsAutoDraw())
		{
			m_reactor.GetModule().Draw();
		}
	}

	void ThumbListBox::AddItem(const std::string& text)
	{
		if (m_reactor.GetModule().AddItem(StringUtils::Convert(text)) && IsAutoDraw())
		{
			m_reactor.GetModule().Draw();
		}
	}

	void ThumbListBox::AddItem(const std::wstring& text, const Image& thumbnail)
	{
		if (m_reactor.GetModule().AddItem(text, thumbnail) && IsAutoDraw())
		{
			m_reactor.GetModule().Draw();
		}
	}

	void ThumbListBox::AddItem(const std::string& text, const Image& thumbnail)
	{
		if (m_reactor.GetModule().AddItem(StringUtils::Convert(text), thumbnail) && IsAutoDraw())
		{
			m_reactor.GetModule().Draw();
		}
	}

	ThumbListBoxItem ThumbListBox::At(size_t index)
	{
		return m_reactor.GetModule().At(index);
	}

	void ThumbListBox::Clear()
	{
		if (m_reactor.GetModule().Clear() && IsAutoDraw())
		{
			m_reactor.GetModule().Draw();
		}
	}

	void ThumbListBox::Erase(size_t index)
	{
		m_reactor.GetModule().Erase(index);
	}

	void ThumbListBox::SetThumbnailSize(uint32_t size)
	{
		m_reactor.GetModule().SetThumbnailSize(size);
	}

	bool ThumbListBox::IsEnabledMultiselection() const
	{
		return m_reactor.GetModule().IsEnabledMultiselection();
	}

	void ThumbListBox::EnableMultiselection(bool enabled)
	{
		if (m_reactor.GetModule().EnableMultiselection(enabled))
		{
			m_reactor.GetModule().ClearSelection();
			m_reactor.GetModule().Draw();
		}
	}

	std::vector<size_t> ThumbListBox::GetSelected() const
	{
		return m_reactor.GetModule().GetSelectedItems();
	}
}
