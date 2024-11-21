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
		m_module.m_appearance = reinterpret_cast<ThumbListBoxAppearance*>(control.Handle()->Appearance.get());

		m_module.m_window = control.Handle();
		m_module.m_control = m_control;
		m_module.CalculateViewport(m_module.m_viewport);
	}

	void ThumbListBoxReactor::Update(Graphics& graphics)
	{
		BT_CORE_TRACE << "  - ThumbListBoxReactor::Update " << std::endl;
		auto window = m_control->Handle();
		bool enabled = m_control->GetEnabled();

		graphics.DrawRectangle(window->Size.ToRectangle(), window->Appearance->BoxBackground, true);

		Point offset{ 0, -m_module.m_state.m_offset };

		auto thumbSize = m_module.m_window->ToScale(m_module.m_thumbnailSize);
		auto cardHeight = m_module.m_window->ToScale(m_module.m_appearance->ThumbnailCardHeight);
		for (size_t i = m_module.m_viewport.m_startingVisibleIndex; i < m_module.m_viewport.m_endingVisibleIndex; i++)
		{
			auto& item = m_module.m_items[i];

			Rectangle cardRect{ item.m_bounds.X + offset.X, item.m_bounds.Y + offset.Y, m_module.m_viewport.m_cardSize.Width, m_module.m_viewport.m_cardSize.Height };
			
			const bool& isSelected = item.m_isSelected;
			bool isLastSelected = (int)i == m_module.m_mouseSelection.m_pressedIndex;
			graphics.DrawRectangle(cardRect, window->Appearance->Background, true);

			Size imageSize = window->ToScale(item.m_thumbnail.GetSize());
			Size thumbFrameSize{ thumbSize, thumbSize };
			auto center = thumbFrameSize - imageSize;
			center *= 0.5f;

			Rectangle thumbnailRect{ cardRect.X + (int)center.Width, cardRect.Y + (int)center.Height, imageSize.Width, imageSize.Height };
			item.m_thumbnail.Paste(graphics, thumbnailRect);

			{
				Size cardTextSize{ thumbSize, cardHeight };
				auto center = cardTextSize - graphics.GetTextExtent(item.m_text);
				center *= 0.5f;

				if (isSelected)
				{
					graphics.DrawRectangle({ cardRect.X , cardRect.Y + (int)thumbSize ,cardRect.Width, cardHeight }, window->Appearance->HighlightColor, true);
				}
				graphics.DrawString({ cardRect.X + (int)center.Width, cardRect.Y + (int)thumbSize + (int)center.Height }, item.m_text, window->Appearance->Foreground);
			}

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
			Graphics selectionBox(boxSize);
			selectionBox.DrawRectangle(blendColor, true);
			selectionBox.DrawRectangle(m_module.m_window->Appearance->SelectionBorderHighlightColor, false);

			Rectangle blendRect{ startPoint.X, startPoint.Y + m_module.m_state.m_offset, boxSize.Width, boxSize.Height};
			graphics.Blend(blendRect, selectionBox, { 0,0 }, 0.5f);
		}

		graphics.DrawRectangle(window->Size.ToRectangle(), enabled ? window->Appearance->BoxBorderColor : window->Appearance->BoxBorderDisabledColor, false);
		//m_module.Draw(graphics);
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
			Rectangle scrollRect{ static_cast<int>(m_module.m_window->Size.Width - scrollSize) - 1, 1, scrollSize, m_module.m_window->Size.Height - 2u };
			GUI::MoveWindow(m_module.m_scrollBar->Handle(), scrollRect);
		}
	}

	void ThumbListBoxReactor::MouseDown(Graphics& graphics, const ArgMouse& args)
	{
		auto itemIndexAtPosition = m_module.GetItemIndexAtMousePosition(args.Position);
		bool hitOnBlank = itemIndexAtPosition == -1;

		bool needUpdate = false;
		m_module.m_mouseSelection.m_pressedIndex = itemIndexAtPosition;

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
			needUpdate = true;
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
				auto pivot = (m_module.m_mouseSelection.m_pressedIndex == -1 ? (direction == -1 ? (int)m_module.m_items.size() : -1) : m_module.m_mouseSelection.m_pressedIndex);
				auto newItemIndex= pivot + direction;
				if (newItemIndex >= 0 && newItemIndex < (int)m_module.m_items.size())
				{
					if (!m_module.m_multiselection || (!m_module.m_shiftPressed && !m_module.m_ctrlPressed))
					{
						m_module.ClearSelection();
					}
					if (m_module.m_shiftPressed || !m_module.m_ctrlPressed)
					{
						m_module.m_items[newItemIndex].m_isSelected = true;
						m_module.m_mouseSelection.m_pressedIndex = newItemIndex;
						m_module.m_mouseSelection.m_selections.push_back(newItemIndex);
						m_module.m_mouseSelection.m_selectedIndex = newItemIndex;
					}
					else if (m_module.m_ctrlPressed)
					{
						m_module.m_mouseSelection.m_pressedIndex += direction;
					}
					m_module.EnsureVisibility(m_module.m_mouseSelection.m_pressedIndex);

					needUpdate = true;
				}
			}
			else if (args.Key == KeyboardKey::ArrowUp || args.Key == KeyboardKey::ArrowDown)
			{
				auto direction = args.Key == KeyboardKey::ArrowUp ? -1 : 1;
				auto pivot = (m_module.m_mouseSelection.m_pressedIndex == -1 ? (direction == -1 ? (int)m_module.m_items.size() : -1) : m_module.m_mouseSelection.m_pressedIndex);
				auto newItemIndex = pivot + direction * m_module.m_viewport.m_totalCardsInRow;
				if (newItemIndex >= 0 && newItemIndex < (int)m_module.m_items.size())
				{
					if (!m_module.m_multiselection || !m_module.m_shiftPressed && !m_module.m_ctrlPressed)
					{
						m_module.ClearSelection();
					}

					if (m_module.m_multiselection && m_module.m_shiftPressed)
					{
						int end = newItemIndex + direction;
						int start = m_module.m_mouseSelection.m_pressedIndex;
						int current = start + direction;
						while (current != end)
						{

							m_module.m_items[current].m_isSelected = true;
							m_module.m_mouseSelection.m_selections.push_back(current);
							current += direction;
						}
						m_module.m_mouseSelection.m_pressedIndex = newItemIndex;
					}
					else if (m_module.m_ctrlPressed)
					{
						m_module.m_mouseSelection.m_pressedIndex = newItemIndex;
					}
					else
					{
						m_module.m_items[newItemIndex].m_isSelected = true;
						m_module.m_mouseSelection.m_selections.push_back(newItemIndex);
						m_module.m_mouseSelection.m_pressedIndex = newItemIndex;
						m_module.m_mouseSelection.m_selectedIndex = newItemIndex;
					}
					m_module.EnsureVisibility(m_module.m_mouseSelection.m_pressedIndex);
					needUpdate = true;
				}
			}
		}
		else if (args.Key == KeyboardKey::Space && m_module.m_ctrlPressed)
		{
			if (m_module.m_mouseSelection.m_pressedIndex != -1)
			{
				auto& isSelected = m_module.m_items[m_module.m_mouseSelection.m_pressedIndex].m_isSelected;
				isSelected = !isSelected;
				if (isSelected)
				{
					m_module.m_mouseSelection.Select(m_module.m_mouseSelection.m_pressedIndex);
				}
				else
				{
					m_module.m_mouseSelection.Deselect(m_module.m_mouseSelection.m_pressedIndex);
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

	void ThumbListBoxReactor::Module::AddItem(const std::wstring& text, const Image& thumbnail)
	{
		auto& newItem = m_items.emplace_back();
		newItem.m_text = text;
		newItem.m_thumbnail = thumbnail;

		BuildItems();
		CalculateVisibleIndices();
	}

	void ThumbListBoxReactor::Module::CalculateViewport(ViewportData& viewportData)
	{
		viewportData.m_backgroundRect = m_window->Size.ToRectangle();
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

	void ThumbListBoxReactor::Module::Clear()
	{
		bool needUpdate = !m_items.empty();
		m_items.clear();

		CalculateViewport(m_viewport);
		CalculateVisibleIndices();

		if (needUpdate)
		{
			if (m_scrollBar)
			{
				m_scrollBar.reset();
				m_state.m_offset = 0;
			}
			GUI::UpdateWindow(m_window);
		}
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
			Rectangle scrollRect{ static_cast<int>(m_window->Size.Width - scrollSize) - 1, 1, scrollSize, m_window->Size.Height - 2u };

			m_scrollBar = std::make_unique<ScrollBar>(m_window, false, scrollRect);
			m_scrollBar->GetEvents().ValueChanged.Connect([this](const ArgScrollBar& args)
				{
					m_state.m_offset = args.Value;
					CalculateVisibleIndices();

					GUI::UpdateWindow(m_window);
				});
		}
		m_scrollBar->SetMinMax(0, (int)(m_viewport.m_contentSize - m_viewport.m_backgroundRect.Height));
		m_scrollBar->SetPageStepValue(m_viewport.m_backgroundRect.Height);
		m_scrollBar->SetStepValue(m_viewport.m_cardSize.Height);

		m_state.m_offset = m_scrollBar->GetValue();
	}

	void ThumbListBoxReactor::Module::EnableMultiselection(bool enabled)
	{
		m_multiselection = enabled;
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
			return true;
		}
		return false;
	}

	bool ThumbListBoxReactor::Module::UpdateSingleSelection(int newItemIndex)
	{
		bool needUpdate = m_mouseSelection.m_selectedIndex != newItemIndex;
		if (needUpdate)
		{
			ClearSingleSelection();
			SelectItem(newItemIndex);
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
		int minIndex = (std::min)(m_mouseSelection.m_selectedIndex, itemIndexAtPosition);
		int maxIndex = (std::max)(m_mouseSelection.m_selectedIndex, itemIndexAtPosition);

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
		m_mouseSelection.m_selectedIndex = itemIndexAtPosition;

		EnsureVisibility(itemIndexAtPosition);
		return needUpdate;
	}

	std::vector<size_t> ThumbListBoxReactor::Module::GetSelectedItems() const
	{
		return m_mouseSelection.m_selections;
	}

	void ThumbListBoxReactor::Module::EnsureVisibility(int lastSelectedIndex)
	{
		if (!m_scrollBar)
		{
			return;
		}

		auto itemBounds = m_items[lastSelectedIndex].m_bounds;
		itemBounds.Y -= m_state.m_offset;

		if (m_viewport.m_backgroundRect.Contains(itemBounds))
		{
			return;
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
		CalculateVisibleIndices();

		m_scrollBar->SetValue(m_state.m_offset);

		GUI::UpdateWindow(m_scrollBar->Handle());
	}

	void ThumbListBoxReactor::Module::UpdatedThumbnail(ItemType& item)
	{
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

	void ThumbListBox::AddItem(const std::wstring& text, const Image& thumbnail)
	{
		m_reactor.GetModule().AddItem(text, thumbnail);
	}

	void ThumbListBox::Clear()
	{
		m_reactor.GetModule().Clear();
	}

	void ThumbListBox::Erase(size_t index)
	{
		m_reactor.GetModule().Erase(index);
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
