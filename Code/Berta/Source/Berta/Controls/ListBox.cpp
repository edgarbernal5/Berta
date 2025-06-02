/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#include "btpch.h"
#include "ListBox.h"

#include "Berta/GUI/Interface.h"
#include "Berta/GUI/EnumTypes.h"

#include <numeric>

namespace Berta
{
	void ListBoxReactor::Init(ControlBase& control)
	{
		m_control = &control;
		m_module.m_window = control.Handle();

		m_module.m_appearance = reinterpret_cast<ListBoxAppearance*>(m_module.m_window->Appearance.get());

		m_module.CalculateViewport(m_module.m_viewport);
		m_module.CalculateVisibleIndices();
	}

	void ListBoxReactor::Update(Graphics& graphics)
	{
		//BT_CORE_TRACE << " -- Listbox Update() " << std::endl;
		auto enabled = m_control->GetEnabled();
		graphics.DrawRectangle(m_module.m_window->ClientSize.ToRectangle(), m_module.m_window->Appearance->BoxBackground, true);

		m_module.DrawList(graphics);
		m_module.DrawHeaders(graphics);

		if (m_module.m_mouseSelection.m_started && m_module.m_mouseSelection.m_startPosition != m_module.m_mouseSelection.m_endPosition)
		{
			Point startPoint, endPoint;
			Size boxSize;
			m_module.CalculateSelectionBox(startPoint, endPoint, boxSize);

			Color blendColor = m_module.m_window->Appearance->SelectionHighlightColor;
			Graphics selectionBox(boxSize, m_module.m_window->DPI, m_module.m_window->RootPaintHandle);
			selectionBox.Begin();
			selectionBox.DrawRectangle(blendColor, true);
			selectionBox.DrawRectangle(m_module.m_window->Appearance->SelectionBorderHighlightColor, false);
			selectionBox.Flush();

			Rectangle blendRect{ startPoint.X + m_module.m_scrollOffset.X, startPoint.Y + m_module.m_scrollOffset.Y, boxSize.Width, boxSize.Height };
			graphics.Blend(blendRect, selectionBox, { 0,0 }, 0.5);
		}

		if (m_module.m_viewport.m_needHorizontalScroll && m_module.m_viewport.m_needVerticalScroll)
		{
			auto scrollSize = m_module.m_window->ToScale(m_module.m_window->Appearance->ScrollBarSize);
			graphics.DrawRectangle({ (int)(m_module.m_window->ClientSize.Width - scrollSize) - 1, (int)(m_module.m_window->ClientSize.Height - scrollSize) - 1, scrollSize, scrollSize }, m_module.m_window->Appearance->Background, true);
		}
		graphics.DrawRectangle(m_module.m_window->ClientSize.ToRectangle(), enabled ? m_module.m_window->Appearance->BoxBorderColor : m_module.m_window->Appearance->BoxBorderDisabledColor, false);
	}

	void ListBoxReactor::DblClick(Graphics& graphics, const ArgMouse& args)
	{
		auto hoveredArea = m_module.DetermineHoverArea(args.Position);
		if (hoveredArea != InteractionArea::HeaderSplitter)
		{
			return;
		}

		auto selectedHeader = m_module.m_headers.m_sorted[m_module.m_headers.m_selectedIndex];
		auto maxCellWidth = 0u;
		for (size_t i = 0; i < m_module.m_list.m_items.size(); i++)
		{
			const auto& cell = m_module.m_list.m_items[i].m_cells[selectedHeader];
			auto cellWidth = graphics.GetTextExtent(cell.m_text).Width;
			if (cellWidth > maxCellWidth)
			{
				maxCellWidth = cellWidth;
			}
		}
		maxCellWidth = m_module.m_window->ToDownwardScale(maxCellWidth);
		auto leftMarginTextHeader = 5u;
		maxCellWidth += leftMarginTextHeader * 2u;

		auto newWidth = (std::max)(maxCellWidth, LISTBOX_MIN_HEADER_WIDTH);
		bool needUpdate = newWidth != m_module.m_headers.m_items[selectedHeader].m_bounds.Width;
		if (!needUpdate)
			return;

		m_module.m_headers.m_items[selectedHeader].m_bounds.Width = newWidth;
		m_module.CalculateViewport(m_module.m_viewport);
		m_module.BuildHeaderBounds(selectedHeader);

		if (m_module.UpdateScrollBars())
		{
			if (m_module.m_scrollBarVert)
				m_module.m_scrollBarVert->Handle()->Renderer.Update();

			if (m_module.m_scrollBarHoriz)
				m_module.m_scrollBarHoriz->Handle()->Renderer.Update();
		}

		hoveredArea = m_module.DetermineHoverArea(args.Position);
		if (hoveredArea != InteractionArea::HeaderSplitter)
		{
			GUI::ChangeCursor(m_module.m_window, Cursor::Default);
		}
		m_module.m_pressedArea = InteractionArea::None;
		m_module.m_hoveredArea = hoveredArea;

		GUI::MarkAsNeedUpdate(m_module.m_window);
	}

	void ListBoxReactor::Resize(Graphics& graphics, const ArgResize& args)
	{
		m_module.CalculateViewport(m_module.m_viewport);
		m_module.CalculateVisibleIndices();

		m_module.UpdateScrollBars();
		m_module.BuildHeaderBounds();
		m_module.BuildListItemBounds();
	}

	void ListBoxReactor::MouseDown(Graphics& graphics, const ArgMouse& args)
	{
		m_module.m_pressedArea = m_module.m_hoveredArea;
		bool needUpdate = false;

		if (m_module.m_pressedArea == InteractionArea::List)
		{
			if (m_module.m_mouseSelection.m_hoveredItem)
			{
				if (m_module.m_multiselection)
				{
					needUpdate = m_module.HandleMultiSelection(m_module.m_mouseSelection.m_hoveredItem, args);
				}
				else
				{
					needUpdate = m_module.UpdateSingleSelection(m_module.m_mouseSelection.m_hoveredItem);
				}
			}
		}
		else if (m_module.m_pressedArea == InteractionArea::ListBlank)
		{
			if (m_module.m_multiselection)
			{
				m_module.StartSelectionRectangle(args.Position);
				needUpdate = m_module.ClearSelectionIfNeeded();
			}
			else
			{
				needUpdate = m_module.ClearSingleSelection();
			}
		}
		else if (m_module.m_pressedArea == InteractionArea::HeaderSplitter)
		{
			m_module.StartHeadersSizing(args.Position);
		}
		else if (m_module.m_pressedArea == InteractionArea::Header)
		{
			m_module.StartSelectingHeader(args.Position);
		}

		if (needUpdate)
		{
			GUI::MarkAsNeedUpdate(m_module.m_window);
		}
	}

	void ListBoxReactor::MouseMove(Graphics& graphics, const ArgMouse& args)
	{
		auto hoveredArea = args.ButtonState.NoButtonsPressed() ? m_module.DetermineHoverArea(args.Position) : m_module.m_pressedArea;
		bool needUpdate = false;

		if ((hoveredArea == InteractionArea::Header || hoveredArea == InteractionArea::HeaderSplitter))
		{
			if (args.ButtonState.NoButtonsPressed())
			{
				needUpdate |= m_module.SetHoveredListItem();

				if (hoveredArea == InteractionArea::HeaderSplitter)
				{
					GUI::ChangeCursor(m_module.m_window, Cursor::SizeWE);
					auto headerSizeIndex = m_module.GetHeaderAtMousePosition(args.Position, true);
					needUpdate = m_module.m_headers.m_selectedIndex != headerSizeIndex;
					m_module.m_headers.m_selectedIndex = headerSizeIndex;
				}
				else
				{
					auto headerSelectedIndex = m_module.GetHeaderAtMousePosition(args.Position, false);
					needUpdate = m_module.m_headers.m_selectedIndex != headerSelectedIndex;
					m_module.m_headers.m_selectedIndex = headerSelectedIndex;
				}
			}
			if (hoveredArea == InteractionArea::HeaderSplitter && args.ButtonState.LeftButton)
			{
				m_module.UpdateHeadersSize(args.Position);

				needUpdate = true;
			}
			else if (hoveredArea == InteractionArea::Header && args.ButtonState.LeftButton)
			{
				auto listItemIconSize = m_module.m_window->ToScale(m_module.m_appearance->ListItemIconSize);
				auto listItemIconMargin = m_module.m_window->ToScale(m_module.m_appearance->ListItemIconMargin);

				if (!m_module.m_headers.m_isDragging && !m_module.m_headers.m_draggingBox.IsValid())
				{
					auto leftMarginTextHeader = m_module.m_window->ToScale(5u);
					auto headerHeight = m_module.m_window->ToScale(m_module.m_appearance->HeadersHeight);

					const auto& headerIndex = m_module.m_headers.m_sorted[m_module.m_headers.m_selectedIndex];
					const auto& header = m_module.m_headers.m_items[headerIndex];
					Graphics& draggingBox = m_module.m_headers.m_draggingBox;
					Rectangle columnRect{ 0,0,m_module.m_window->ToScale(header.m_bounds.Width), headerHeight };
					uint32_t textOffset = 0;
					if (m_module.m_headers.m_selectedIndex == 0 && m_module.m_list.m_drawImages)
					{
						textOffset += listItemIconSize + listItemIconMargin * 2u;
						columnRect.Width += (listItemIconSize + listItemIconMargin * 2u);
					}

					draggingBox.Build({ columnRect.Width, columnRect.Height }, m_module.m_window->RootPaintHandle);
					draggingBox.BuildFont(m_module.m_window->DPI);

					draggingBox.Begin();
					draggingBox.DrawGradientFill({ 0,0, columnRect.Width, columnRect.Height }, m_module.m_appearance->Foreground, m_module.m_appearance->Foreground2nd);
					
					Rectangle textRect = columnRect;
					textRect.X += (int)leftMarginTextHeader + textOffset;
					textRect.Width -= leftMarginTextHeader * 2 + textOffset;
					m_module.DrawHeaderItem(draggingBox, { 0,0,columnRect.Width ,columnRect.Height }, header.m_name, false, textRect, m_module.m_appearance->SelectionHighlightColor);
					
					draggingBox.Flush();
				}
				m_module.m_headers.m_mouseDraggingPosition = args.Position.X;
				m_module.m_headers.m_isDragging = true;

				auto mousePositionX = args.Position.X + m_module.m_scrollOffset.X - (int)m_module.m_viewport.m_columnOffsetStartOff;
				auto targetHeaderIndex = m_module.GetHeaderAtMousePosition(args.Position, false);

				if (targetHeaderIndex != -1)
				{
					const auto& headerIndex = m_module.m_headers.m_sorted[targetHeaderIndex];
					const auto& headerItem = m_module.m_headers.m_items[headerIndex];
					auto headerPosX = m_module.m_window->ToScale(headerItem.m_bounds.X);
					auto headerWidth = (int)m_module.m_window->ToScale(headerItem.m_bounds.Width);
					if (targetHeaderIndex == 0 && m_module.m_list.m_drawImages)
					{
						headerWidth += (int)(listItemIconSize + listItemIconMargin * 2u);
					}
					if (targetHeaderIndex > 0 && m_module.m_list.m_drawImages)
					{
						headerPosX += (int)(listItemIconSize + listItemIconMargin * 2u);
					}
					auto headerHalfWidth = headerWidth >> 1;
					if (mousePositionX >= headerPosX + headerHalfWidth && mousePositionX <= headerPosX + headerWidth)
					{
						targetHeaderIndex++;
					}
				}
				else if (mousePositionX <= (int)m_module.m_viewport.m_columnOffsetStartOff)
				{
					targetHeaderIndex = 0;
				}
				else
				{
					targetHeaderIndex = (int)m_module.m_headers.m_items.size();
				}
				m_module.m_headers.m_draggingTargetIndex = targetHeaderIndex;
				needUpdate = true;
			}
		}
		else if (hoveredArea == InteractionArea::List)
		{
			if (args.ButtonState.NoButtonsPressed())
			{
				auto itemHeight = m_module.m_window->ToScale(m_module.m_appearance->ListItemHeight) + m_module.m_viewport.m_innerMargin * 2u;

				auto positionY = args.Position.Y - m_module.m_viewport.m_backgroundRect.Y + m_module.m_scrollOffset.Y;
				int newIndex = positionY / (int)itemHeight;
				auto absIndex = m_module.m_list.m_sortedIndexes[newIndex];

				needUpdate |= m_module.SetHoveredListItem(&m_module.m_list.m_items[absIndex]) && !m_module.m_list.m_items[absIndex].m_isSelected;
			}
		}
		else if (hoveredArea == InteractionArea::ListBlank || hoveredArea == InteractionArea::None)
		{
			needUpdate |= m_module.SetHoveredListItem();

			if (m_module.m_mouseSelection.m_started)
			{
				auto logicalPosition = args.Position;
				logicalPosition -= m_module.m_scrollOffset;
				m_module.m_mouseSelection.m_endPosition = logicalPosition;

				Point startPoint, endPoint;
				Size boxSize;
				m_module.CalculateSelectionBox(startPoint, endPoint, boxSize);

				needUpdate |= (boxSize.Width > 0 && boxSize.Height > 0);
				if (boxSize.Width > 0 && boxSize.Height > 0)
				{
					Rectangle selectionRect{ startPoint.X + m_module.m_scrollOffset.X, startPoint.Y + m_module.m_scrollOffset.Y * 2 - m_module.m_viewport.m_backgroundRect.Y, boxSize.Width, boxSize.Height };

					for (size_t i = m_module.m_viewport.m_startingVisibleIndex; i < m_module.m_viewport.m_endingVisibleIndex; i++)
					{
						auto absoluteIndex = m_module.m_list.m_sortedIndexes[i];
						auto& item = m_module.m_list.m_items[absoluteIndex];
						item.m_bounds.Y = (int)((m_module.m_viewport.m_itemHeightWithMargin * i) + m_module.m_viewport.m_innerMargin);
						item.m_bounds.Width = m_module.m_viewport.m_contentSize.Width;
						bool intersection = item.m_bounds.Intersect(selectionRect);
						bool alreadySelected = m_module.m_mouseSelection.IsAlreadySelected(&item);

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
		}
		if (args.ButtonState.NoButtonsPressed() && hoveredArea != InteractionArea::HeaderSplitter && hoveredArea != InteractionArea::Header
			&& m_module.m_headers.m_selectedIndex != -1)
		{
			m_module.m_headers.m_selectedIndex = -1;
			needUpdate = true;
		}
		if (args.ButtonState.NoButtonsPressed() && hoveredArea != InteractionArea::HeaderSplitter && m_module.m_hoveredArea == InteractionArea::HeaderSplitter)
		{
			GUI::ChangeCursor(m_module.m_window, Cursor::Default);
		}

		m_module.m_hoveredArea = hoveredArea;
		if (needUpdate)
		{
			GUI::MarkAsNeedUpdate(m_module.m_window);
		}
	}

	void ListBoxReactor::MouseUp(Graphics& graphics, const ArgMouse& args)
	{
		bool needUpdate = false;

		if (m_module.m_pressedArea == InteractionArea::HeaderSplitter &&
			m_module.DetermineHoverArea(args.Position) != InteractionArea::HeaderSplitter)
		{
			GUI::ChangeCursor(m_module.m_window, Cursor::Default);
		}

		if (m_module.m_mouseSelection.m_started)
		{
			Point startPoint, endPoint;
			Size boxSize;
			m_module.CalculateSelectionBox(startPoint, endPoint, boxSize);
			needUpdate = (boxSize.Width > 0 && boxSize.Height > 0);

			m_module.m_mouseSelection.m_started = false;
			m_module.m_mouseSelection.m_selections.clear();
			for (size_t i = 0; i < m_module.m_list.m_items.size(); i++)
			{
				if (m_module.m_list.m_items[i].m_isSelected)
				{
					m_module.m_mouseSelection.m_selections.push_back(&m_module.m_list.m_items[i]);
				}
			}
			GUI::ReleaseCapture(m_module.m_window);
		}
		else if (m_module.m_pressedArea == InteractionArea::HeaderSplitter)
		{
			m_module.StopHeadersSizing();
		}
		else if (m_module.m_pressedArea == InteractionArea::Header)
		{
			m_module.StopDragOrSortHeader();
			needUpdate = true;
		}

		m_module.m_pressedArea = InteractionArea::None;
		if (needUpdate)
		{
			GUI::MarkAsNeedUpdate(*m_control);
		}
	}

	void ListBoxReactor::MouseLeave(Graphics& graphics, const ArgMouse& args)
	{
		if (args.ButtonState.NoButtonsPressed() && m_module.m_hoveredArea == InteractionArea::HeaderSplitter)
		{
			GUI::ChangeCursor(m_module.m_window, Cursor::Default);
		}

		bool needUpdate = m_module.m_mouseSelection.m_hoveredItem != nullptr;
		m_module.m_mouseSelection.m_hoveredItem = nullptr;
		if (args.ButtonState.NoButtonsPressed() && (m_module.m_hoveredArea == InteractionArea::Header || m_module.m_hoveredArea == InteractionArea::HeaderSplitter)
			&& m_module.m_headers.m_selectedIndex != -1)
		{
			m_module.m_headers.m_selectedIndex = -1;
			needUpdate = true;
		}

		m_module.m_hoveredArea = InteractionArea::None;
		if (needUpdate)
		{
			GUI::MarkAsNeedUpdate(m_module.m_window);
		}
	}

	void ListBoxReactor::MouseWheel(Graphics& graphics, const ArgWheel& args)
	{
		if (!m_module.m_scrollBarVert && args.IsVertical || !m_module.m_scrollBarHoriz && !args.IsVertical)
		{
			return;
		}

		int direction = args.WheelDelta > 0 ? -1 : 1;
		direction *= args.IsVertical ? m_module.m_scrollBarVert->GetStepValue() : m_module.m_scrollBarHoriz->GetStepValue();
		auto min = args.IsVertical ? m_module.m_scrollBarVert->GetMin() : m_module.m_scrollBarHoriz->GetMin();
		auto max = args.IsVertical ? m_module.m_scrollBarVert->GetMax() : m_module.m_scrollBarHoriz->GetMax();
		int newOffset = std::clamp((args.IsVertical ? m_module.m_scrollOffset.Y : m_module.m_scrollOffset.X) + direction, (int)min, (int)max);

		if (args.IsVertical && newOffset != m_module.m_scrollOffset.Y ||
			!args.IsVertical && newOffset != m_module.m_scrollOffset.X)
		{
			if (args.IsVertical)
			{
				m_module.m_scrollOffset.Y = newOffset;
				m_module.CalculateVisibleIndices();
				m_module.m_scrollBarVert->SetValue(newOffset);

				m_module.m_scrollBarVert->Handle()->Renderer.Update();
				GUI::MarkAsNeedUpdate(m_module.m_scrollBarVert->Handle());
			}
			else
			{
				m_module.m_scrollOffset.X = newOffset;
				m_module.m_scrollBarHoriz->SetValue(newOffset);

				GUI::MarkAsNeedUpdate(m_module.m_scrollBarHoriz->Handle());
			}

			GUI::MarkAsNeedUpdate(*m_control);
		}
	}

	void ListBoxReactor::KeyPressed(Graphics& graphics, const ArgKeyboard& args)
	{
		m_module.m_shiftPressed = m_module.m_shiftPressed || args.Key == KeyboardKey::Shift;
		m_module.m_ctrlPressed = m_module.m_ctrlPressed || args.Key == KeyboardKey::Control;

		bool needUpdate = false;
		if (args.Key == KeyboardKey::ArrowUp || args.Key == KeyboardKey::ArrowDown)
		{
			auto direction = args.Key == KeyboardKey::ArrowUp ? -1 : 1;

			int selectedIndex = -1;
			if (m_module.m_mouseSelection.m_selectedItem)
			{
				selectedIndex = m_module.GetListItemIndex(m_module.m_mouseSelection.m_selectedItem);
			}
			else
			{
				selectedIndex = (direction == -1 ? (int)m_module.m_list.m_items.size() : -1);
			}

			auto newItemIndex = selectedIndex + direction;
			if (newItemIndex >= 0 && newItemIndex < (int)m_module.m_list.m_items.size())
			{
				auto absoluteIndex = m_module.m_list.m_sortedIndexes[newItemIndex];
				auto newItemPtr = &m_module.m_list.m_items[m_module.m_list.m_sortedIndexes[newItemIndex]];
				if (!m_module.m_ctrlPressed)
				{
					m_module.ClearSelection();
				}

				if (m_module.m_multiselection && m_module.m_shiftPressed && m_module.m_mouseSelection.m_pivotItem)
				{
					int pivotIndex = m_module.GetListItemIndex(m_module.m_mouseSelection.m_pivotItem);

					int endIndex = newItemIndex;
					int startIndex = pivotIndex;
					int minIndex = (std::min)(startIndex, endIndex);
					int maxIndex = (std::max)(startIndex, endIndex);
					
					for (int current = minIndex; current <= maxIndex; ++current)
					{
						auto absoluteIndex = m_module.m_list.m_sortedIndexes[current];
						if (!m_module.m_list.m_items[absoluteIndex].m_isSelected)
						{
							m_module.m_list.m_items[absoluteIndex].m_isSelected = true;
							m_module.m_mouseSelection.m_selections.push_back(&m_module.m_list.m_items[absoluteIndex]);
						}

					}
					m_module.m_mouseSelection.m_selectedItem = newItemPtr;
				}
				else if (m_module.m_ctrlPressed)
				{
					m_module.m_mouseSelection.m_selectedItem = newItemPtr;
				}
				else
				{
					newItemPtr->m_isSelected = true;
					m_module.m_mouseSelection.m_selections.push_back(newItemPtr);
					m_module.m_mouseSelection.m_selectedItem = newItemPtr;
					m_module.m_mouseSelection.m_pivotItem = newItemPtr;
				}
				m_module.EnsureVisibility(newItemIndex);
				needUpdate = true;
			}
		}
		else if (args.Key == KeyboardKey::Space && m_module.m_ctrlPressed)
		{
			if (m_module.m_mouseSelection.m_selectedItem)
			{
				if (!m_module.m_multiselection && !m_module.m_mouseSelection.m_selections.empty())
				{
					m_module.m_mouseSelection.m_selections[0]->m_isSelected = false;
					m_module.m_mouseSelection.m_selections.clear();
				}

				auto& isSelected = m_module.m_mouseSelection.m_selectedItem->m_isSelected;
				isSelected = !isSelected;
				if (isSelected)
				{
					m_module.m_mouseSelection.Select(m_module.m_mouseSelection.m_selectedItem);
				}
				else
				{
					m_module.m_mouseSelection.Deselect(m_module.m_mouseSelection.m_selectedItem);
				}

				if (m_module.m_multiselection)
				{
					m_module.m_mouseSelection.m_pivotItem = m_module.m_mouseSelection.m_selectedItem;
				}
				
				needUpdate = true;
			}
		}

		if (needUpdate)
		{
			GUI::MarkAsNeedUpdate(*m_control);
		}
	}

	void ListBoxReactor::KeyReleased(Graphics& graphics, const ArgKeyboard& args)
	{
		if (args.Key == KeyboardKey::Shift) m_module.m_shiftPressed = false;
		if (args.Key == KeyboardKey::Control) m_module.m_ctrlPressed = false;
	}

	void ListBoxReactor::Module::CalculateViewport(ViewportData& viewportData)
	{
		viewportData.m_backgroundRect = m_window->ClientSize.ToRectangle();
		viewportData.m_backgroundRect.Y = viewportData.m_backgroundRect.X = 1;
		viewportData.m_backgroundRect.Width -= 2u;
		viewportData.m_backgroundRect.Height -= 2u;
		viewportData.m_innerMargin = m_window->ToScale(2u);
		viewportData.m_columnOffsetStartOff = m_window->ToScale(4u);
		viewportData.m_backgroundRect.Width -= viewportData.m_columnOffsetStartOff;

		auto headerHeight = m_window->ToScale(m_appearance->HeadersHeight);
		viewportData.m_itemHeight = m_window->ToScale(m_appearance->ListItemHeight);
		viewportData.m_itemHeightWithMargin = viewportData.m_itemHeight + viewportData.m_innerMargin * 2u;

		viewportData.m_backgroundRect.Y += headerHeight;
		viewportData.m_backgroundRect.Height -= headerHeight;

		viewportData.m_contentSize.Height = (uint32_t)m_list.m_items.size() * (viewportData.m_itemHeightWithMargin);

		viewportData.m_contentSize.Width = viewportData.m_columnOffsetStartOff;
		if (m_list.m_drawImages)
		{
			auto listItemIconSize = m_window->ToScale(m_appearance->ListItemIconSize);
			auto listItemIconMargin = m_window->ToScale(m_appearance->ListItemIconMargin);
			viewportData.m_contentSize.Width += listItemIconSize + listItemIconMargin * 2u;
		}
		for (size_t i = 0; i < m_headers.m_items.size(); i++)
		{
			auto headerWidth = m_window->ToScale(m_headers.m_items[i].m_bounds.Width);
			viewportData.m_contentSize.Width += headerWidth;
		}

		auto scrollSize = m_window->ToScale(m_window->Appearance->ScrollBarSize);
		viewportData.m_needVerticalScroll = viewportData.m_contentSize.Height > viewportData.m_backgroundRect.Height;
		if (viewportData.m_needVerticalScroll)
		{
			viewportData.m_backgroundRect.Width -= scrollSize;
		}
		viewportData.m_needHorizontalScroll = viewportData.m_contentSize.Width > viewportData.m_backgroundRect.Width;
		if (viewportData.m_needHorizontalScroll)
		{
			viewportData.m_backgroundRect.Height -= scrollSize;
			if (!viewportData.m_needVerticalScroll)
			{
				viewportData.m_needVerticalScroll = viewportData.m_contentSize.Height > viewportData.m_backgroundRect.Height;
				if (viewportData.m_needVerticalScroll)
				{
					viewportData.m_backgroundRect.Width -= scrollSize;
				}
			}
		} 
	}

	void ListBoxReactor::Module::CalculateVisibleIndices()
	{
		if (m_list.m_items.empty() || !m_scrollBarVert)
		{
			m_viewport.m_startingVisibleIndex = 0;
			m_viewport.m_endingVisibleIndex = (int)m_list.m_items.size();
			return;
		}

		int viewportHeight = (int)(m_viewport.m_backgroundRect.Height);
		int itemHeightWithMargin = (int)(m_viewport.m_itemHeightWithMargin);
		int startRow = m_scrollOffset.Y / itemHeightWithMargin;
		int endRow = 1 + (m_scrollOffset.Y + viewportHeight) / itemHeightWithMargin;

		m_viewport.m_startingVisibleIndex = startRow;
		m_viewport.m_endingVisibleIndex = (std::min)(endRow, (int)m_list.m_items.size());
	}

	void ListBoxReactor::Module::Erase(ListBoxItem item)
	{
		auto itemPtr = item.m_target;
		bool wasSelected = m_mouseSelection.IsSelected(itemPtr);
		if (wasSelected)
		{
			itemPtr->m_isSelected = false;
			m_mouseSelection.Deselect(itemPtr);
			m_mouseSelection.ClearReferences(itemPtr);
		}
		auto localIndex = GetListItemIndex(itemPtr);
		auto index = m_list.m_sortedIndexes[localIndex];
		m_list.m_items.erase(m_list.m_items.begin() + index);

		auto it = std::find(m_list.m_sortedIndexes.begin(), m_list.m_sortedIndexes.end(), index);
		if (it != m_list.m_sortedIndexes.end())
		{
			m_list.m_sortedIndexes.erase(it);
		}

		for (auto& sortedIndex : m_list.m_sortedIndexes)
		{
			if (sortedIndex > index)
			{
				--sortedIndex;
			}
		}

		m_mouseSelection.m_selections.clear();
		for (auto& item : m_list.m_items)
		{
			if (item.m_isSelected)
			{
				m_mouseSelection.m_selections.push_back(&item);
			}
		}

		CalculateViewport(m_viewport);
		CalculateVisibleIndices();
		UpdateScrollBars();

		if (index < m_list.m_items.size())
		{
			BuildListItemBounds(index);
		}

		if (m_viewport.m_needVerticalScroll)
		{
			m_scrollBarVert->Handle()->Renderer.Update();
			//GUI::UpdateWindow(m_scrollBarVert->Handle());
		}
		if (m_viewport.m_needHorizontalScroll)
		{
			m_scrollBarHoriz->Handle()->Renderer.Update();
			//GUI::UpdateWindow(m_scrollBarHoriz->Handle());
		}

		GUI::UpdateWindow(m_window);
	}

	void ListBoxReactor::Module::Erase(std::vector<ListBoxItem>& items)
	{
		if (items.empty())
			return;

		std::vector<size_t> deferredSortedErase(items.size());

		int minIndex = (std::numeric_limits<int>::max)();
		size_t i = 0;
		for (auto& item : items)
		{
			auto itemPtr = item.m_target;
			bool wasSelected = m_mouseSelection.IsSelected(itemPtr);
			if (wasSelected)
			{
				itemPtr->m_isSelected = false;
				m_mouseSelection.Deselect(itemPtr);
				m_mouseSelection.ClearReferences(itemPtr);
			}
			auto index = GetListItemIndex(itemPtr);
			minIndex = (std::min)(minIndex, index);

			deferredSortedErase[i] = m_list.m_sortedIndexes[index];

			++i;
		}

		std::sort(deferredSortedErase.rbegin(), deferredSortedErase.rend());
		for (size_t index : deferredSortedErase)
		{
			m_list.m_items.erase(m_list.m_items.begin() + index);

			auto it = std::find(m_list.m_sortedIndexes.begin(), m_list.m_sortedIndexes.end(), index);
			if (it != m_list.m_sortedIndexes.end())
			{
				m_list.m_sortedIndexes.erase(it);
			}

			for (auto& sortedIndex : m_list.m_sortedIndexes)
			{
				if (sortedIndex > index)
				{
					--sortedIndex;
				}
			}
		}

		m_mouseSelection.m_selections.clear();
		for (auto& item : m_list.m_items)
		{
			if (item.m_isSelected)
			{
				m_mouseSelection.m_selections.push_back(&item);
			}
		}

		CalculateViewport(m_viewport);
		CalculateVisibleIndices();
		UpdateScrollBars();

		if (minIndex < m_list.m_items.size())
		{
			BuildListItemBounds(minIndex);
		}

		if (m_viewport.m_needVerticalScroll)
		{
			m_scrollBarVert->Handle()->Renderer.Update();
			//GUI::UpdateWindow(m_scrollBarVert->Handle());
		}
		if (m_viewport.m_needHorizontalScroll)
		{
			m_scrollBarHoriz->Handle()->Renderer.Update();
			//GUI::UpdateWindow(m_scrollBarHoriz->Handle());
		}

		GUI::UpdateWindow(m_window);
	}

	void ListBoxReactor::Module::EnableMultiselection(bool enabled)
	{
		m_multiselection = enabled;
	}

	void ListBoxReactor::Module::BuildHeaderBounds(size_t startIndex)
	{
		Point offset{ 0,0 };
		if (startIndex > 0)
		{
			const auto& headerIndex = m_headers.m_sorted[startIndex - 1];
			offset.X = m_headers.m_items[headerIndex].m_bounds.X + (int)m_headers.m_items[headerIndex].m_bounds.Width;
		}
		for (size_t i = startIndex; i < m_headers.m_items.size(); i++)
		{
			const auto& headerIndex = m_headers.m_sorted[i];
			m_headers.m_items[headerIndex].m_bounds.X = offset.X;

			offset.X += m_headers.m_items[headerIndex].m_bounds.Width;
		}
	}

	void ListBoxReactor::Module::BuildListItemBounds(size_t startIndex)
	{
		auto listItemHeight = m_window->ToScale(m_appearance->ListItemHeight);
		auto innerMarginInt = static_cast<int>(m_viewport.m_innerMargin);
		Point offset{ 0,innerMarginInt };

		if (startIndex > 0)
		{
			offset.Y = m_list.m_items[startIndex - 1].m_bounds.Y + (int)m_list.m_items[startIndex - 1].m_bounds.Height + innerMarginInt;
		}

		for (size_t i = startIndex; i < m_list.m_items.size(); i++)
		{
			m_list.m_items[i].m_bounds.X = offset.X;
			m_list.m_items[i].m_bounds.Y = offset.Y;
			m_list.m_items[i].m_bounds.Height = listItemHeight;
			m_list.m_items[i].m_bounds.Width = m_viewport.m_contentSize.Width;

			offset.Y += (int)m_list.m_items[i].m_bounds.Height + innerMarginInt * 2;
		}
	}

	void ListBoxReactor::Module::DrawStringInBox(Graphics& graphics, const std::string& str, const Rectangle& boxBounds, const Color& textColor)
	{
		auto textExtent = graphics.GetTextExtent(str);
		if (boxBounds.X + (int)textExtent.Width < 0)
		{
			return;
		}

		if (textExtent.Width < boxBounds.Width)
		{
			graphics.DrawString({ boxBounds.X, boxBounds.Y + ((int)(boxBounds.Height - textExtent.Height) >> 1) }, str, textColor);
			return;
		}

		auto ellipsisTextExtent = graphics.GetTextExtent("...").Width;
		for (size_t i = str.size(); i >= 1; --i)
		{
			auto subStr = str.substr(0, i);// +"...";
			auto subTextExtent = graphics.GetTextExtent(subStr).Width;
			if ((int)(subTextExtent + ellipsisTextExtent) <= (int)boxBounds.Width - 2)
			{
				graphics.DrawString({ boxBounds.X, boxBounds.Y + ((int)(boxBounds.Height - textExtent.Height) >> 1) }, subStr + "...", textColor);
				break;
			}
		}
	}

	void ListBoxReactor::Module::AppendHeader(const std::string& text, uint32_t width)
	{
		auto startIndex = m_headers.m_items.size();
		m_headers.m_items.emplace_back(text, (std::max)(width, LISTBOX_MIN_HEADER_WIDTH));
		m_headers.m_sorted.emplace_back(m_headers.m_sorted.size());

		CalculateViewport(m_viewport); 
		CalculateVisibleIndices();
		BuildHeaderBounds(startIndex);
	}

	ListBoxItem ListBoxReactor::Module::Append(const std::string& text)
	{
		auto startIndex = m_list.m_items.size();
		m_list.m_items.emplace_back(text);
		m_list.m_sortedIndexes.emplace_back(startIndex);

		for (size_t i = 1; i < m_headers.m_items.size(); i++)
		{
			m_list.m_items.back().m_cells.emplace_back("");
		}
		if (m_headers.m_sortedHeaderIndex != -1)
		{
			size_t selectedHeaderIndex = static_cast<size_t>(m_headers.m_sortedHeaderIndex);

			SortHeader(m_headers.m_sorted[selectedHeaderIndex], m_headers.isAscendingOrdering);
		}

		CalculateViewport(m_viewport);
		CalculateVisibleIndices();
		BuildListItemBounds(startIndex);
		UpdateScrollBars();

		GUI::UpdateWindow(m_window);

		return { &m_list.m_items.back(), this};
	}

	ListBoxItem ListBoxReactor::Module::Append(std::initializer_list<std::string> texts)
	{
		auto startIndex = m_list.m_items.size();

		auto headersCount = m_headers.m_items.size();
		auto& item = m_list.m_items.emplace_back("{}");
		size_t position = 0;
		for (auto& text : texts)
		{
			if (item.m_cells.size() == position)
			{
				item.m_cells.emplace_back(text);
			}
			else
			{
				item.m_cells[position] = text;
			}
			++position;
			if (position >= headersCount)
			{
				break;
			}
		}
		m_list.m_sortedIndexes.emplace_back(startIndex);

		if (m_headers.m_sortedHeaderIndex != -1)
		{
			size_t selectedHeaderIndex = static_cast<size_t>(m_headers.m_sortedHeaderIndex);

			SortHeader(m_headers.m_sorted[selectedHeaderIndex], m_headers.isAscendingOrdering);
		}

		CalculateViewport(m_viewport);
		CalculateVisibleIndices();
		BuildListItemBounds(startIndex);

		return { &m_list.m_items.back(), this };
	}

	ListBoxItem ListBoxReactor::Module::At(size_t index)
	{
		auto wrapper = &m_list.m_items[index];
		return ListBoxItem{ wrapper, this };
	}

	void ListBoxReactor::Module::Clear()
	{
		bool needUpdate = !m_list.m_items.empty();
		m_list.m_items.clear();
		m_list.m_sortedIndexes.clear();

		m_mouseSelection.Clear();
		CalculateViewport(m_viewport);
		CalculateVisibleIndices();
		if (UpdateScrollBars())
		{
			if (m_viewport.m_needHorizontalScroll)
			{
				m_scrollBarHoriz->Handle()->Renderer.Update();
			}
		}

		if (needUpdate)
		{
			GUI::UpdateWindow(m_window);
		}
	}

	void ListBoxReactor::Module::ClearHeaders()
	{
		bool needUpdate = !m_headers.m_items.empty();
		m_headers.m_items.clear();
		m_headers.m_sorted.clear();
		m_list.m_items.clear();

		m_headers.m_selectedIndex = -1;
		m_headers.m_draggingTargetIndex = -1;

		if (needUpdate)
		{
			GUI::UpdateWindow(m_window);
		}
	}

	bool ListBoxReactor::Module::UpdateScrollBars()
	{
		auto scrollSize = m_window->ToScale(m_window->Appearance->ScrollBarSize);
		bool needUpdate = false;

		if (m_viewport.m_needVerticalScroll)
		{
			Rectangle scrollRect{ static_cast<int>(m_window->ClientSize.Width - scrollSize) - 1, 1, scrollSize, m_window->ClientSize.Height - 2u };
			if (m_viewport.m_needHorizontalScroll)
			{
				scrollRect.Height -= scrollSize;
			}

			if (!m_scrollBarVert)
			{
				m_scrollBarVert = std::make_unique<ScrollBar>(m_window, false, scrollRect);
				m_scrollBarVert->GetEvents().ValueChanged.Connect([this](const ArgScrollBar& args)
					{
						m_scrollOffset.Y = args.Value;
						CalculateVisibleIndices();

						GUI::UpdateWindow(m_window);
					});
			}
			else
			{
				GUI::MoveWindow(m_scrollBarVert->Handle(), scrollRect);
			}

			auto listItemHeight = m_window->ToScale(m_appearance->ListItemHeight) + m_viewport.m_innerMargin * 2u;
			m_scrollBarVert->SetMinMax(0, (int)(m_viewport.m_contentSize.Height - m_viewport.m_backgroundRect.Height));
			m_scrollBarVert->SetPageStepValue(m_viewport.m_backgroundRect.Height);
			m_scrollBarVert->SetStepValue(listItemHeight);

			m_scrollOffset.Y = m_scrollBarVert->GetValue();
			CalculateVisibleIndices();
			needUpdate = true;
		}
		else if (m_scrollBarVert)
		{
			m_scrollBarVert.reset();
			m_scrollOffset.Y = 0;
			CalculateVisibleIndices();

			needUpdate = true;
		}

		if (m_viewport.m_needHorizontalScroll)
		{
			Rectangle scrollRect{ 1, static_cast<int>(m_window->ClientSize.Height - scrollSize) - 1, m_window->ClientSize.Width - 2u, scrollSize };
			if (m_viewport.m_needVerticalScroll)
			{
				scrollRect.Width -= scrollSize;
			}

			if (!m_scrollBarHoriz)
			{
				m_scrollBarHoriz = std::make_unique<ScrollBar>(m_window, false, scrollRect, false);
				m_scrollBarHoriz->GetEvents().ValueChanged.Connect([this](const ArgScrollBar& args)
					{
						m_scrollOffset.X = args.Value;

						GUI::UpdateWindow(m_window);
					});
			}
			else
			{
				GUI::MoveWindow(m_scrollBarHoriz->Handle(), scrollRect);
			}

			m_scrollBarHoriz->SetMinMax(0, (int)(m_viewport.m_contentSize.Width - m_viewport.m_backgroundRect.Width));
			m_scrollBarHoriz->SetPageStepValue(m_viewport.m_backgroundRect.Width);
			m_scrollBarHoriz->SetStepValue(scrollSize);

			m_scrollOffset.X = m_scrollBarHoriz->GetValue();
			needUpdate = true;
		}
		else if (!m_viewport.m_needHorizontalScroll && m_scrollBarHoriz)
		{
			m_scrollBarHoriz.reset();
			m_scrollOffset.X = 0;

			needUpdate = true;
		}

		return needUpdate;
	}

	void ListBoxReactor::Module::DrawHeaders(Graphics& graphics)
	{
		auto headerHeight = m_window->ToScale(m_appearance->HeadersHeight);
		auto leftMarginTextHeader = m_window->ToScale(5u);
		auto listItemIconSize = m_window->ToScale(m_appearance->ListItemIconSize);
		auto listItemIconMargin = m_window->ToScale(m_appearance->ListItemIconMargin);

		int sortedHeaderMargin = m_window->ToScale(4);
		int arrowSortedHeaderSize = m_window->ToScale(6);
		graphics.DrawGradientFill({ 0,0, m_window->ClientSize.Width, headerHeight }, m_appearance->ButtonHighlightBackground, m_appearance->ButtonBackground);
		graphics.DrawLine({ m_viewport.m_backgroundRect.X + (int)m_viewport.m_columnOffsetStartOff - m_scrollOffset.X - 1, 0 }, { m_viewport.m_backgroundRect.X + (int)m_viewport.m_columnOffsetStartOff - m_scrollOffset.X - 1, (int)headerHeight - 1 }, m_appearance->BoxBorderColor);
		
		Point headerOffset{ m_viewport.m_backgroundRect.X + (int)m_viewport.m_columnOffsetStartOff - m_scrollOffset.X, m_viewport.m_backgroundRect.Y };

		for (size_t i = 0; i < m_headers.m_items.size(); ++i)
		{
			const auto& headerIndex = m_headers.m_sorted[i];
			const auto& header = m_headers.m_items[headerIndex];
			auto headerWidth = m_window->ToScale(header.m_bounds.Width);
			if (i == 0 && m_list.m_drawImages)
			{
				headerWidth += listItemIconSize + listItemIconMargin * 2u;
			}
			auto headerWidthInt = (int)headerWidth;
			bool isHovered = m_headers.m_selectedIndex == (int)i;
			bool isDragging = m_headers.m_isDragging && isHovered;
			if (headerOffset.X + headerWidthInt < 0 || headerOffset.X >= (int)m_viewport.m_backgroundRect.Width)
			{
				headerOffset.X += headerWidthInt;
				continue;
			}
			int textOffset = 0;
			if (i == 0 && m_list.m_drawImages)
			{
				textOffset += listItemIconSize + listItemIconMargin * 2u;
			}
			Rectangle columnRect{ headerOffset.X, 0, headerWidth, headerHeight };
			bool isSortedHeader = headerIndex == m_headers.m_sortedHeaderIndex;
			Rectangle textRect = columnRect;
			textRect.X += (int)leftMarginTextHeader + textOffset;
			textRect.Width -= leftMarginTextHeader * 2 + textOffset;
			if (isSortedHeader)
			{
				textRect.Width -= sortedHeaderMargin + arrowSortedHeaderSize;
			}
			DrawHeaderItem(graphics, columnRect, header.m_name, isHovered, textRect, m_appearance->Foreground);

			if (isDragging)
			{
				int lineWidth = m_window->ToScale(2);
				auto targetHeaderPosition = 0;
				if (m_headers.m_draggingTargetIndex < m_headers.m_items.size())
				{
					const auto& headerIndex = m_headers.m_sorted[m_headers.m_draggingTargetIndex];
					targetHeaderPosition = m_window->ToScale(m_headers.m_items[headerIndex].m_bounds.X);
				}
				else
				{
					const auto& headerIndex = m_headers.m_sorted[m_headers.m_items.size() - 1];
					const auto& lastHeaderBounds = m_headers.m_items[headerIndex].m_bounds;
					targetHeaderPosition = m_window->ToScale(lastHeaderBounds.X + lastHeaderBounds.Width);
				}
				targetHeaderPosition += m_viewport.m_backgroundRect.X + (int)m_viewport.m_columnOffsetStartOff - m_scrollOffset.X;
				if (m_headers.m_draggingTargetIndex != 0 && m_list.m_drawImages)
				{
					targetHeaderPosition += (int)(listItemIconSize + listItemIconMargin * 2u);
				}
				graphics.DrawLine({ targetHeaderPosition, 0 }, { targetHeaderPosition, (int)headerHeight - lineWidth }, lineWidth, m_appearance->SelectionHighlightColor);

				Graphics& draggingBox = m_headers.m_draggingBox;
				
				auto headerPosition = m_headers.m_items[headerIndex].m_bounds.X;
				auto newPosition = m_headers.m_mouseDraggingPosition - m_headers.m_mouseDownOffset;
				if (m_headers.m_selectedIndex != 0 && m_list.m_drawImages)
				{
					newPosition += (int)(listItemIconSize + listItemIconMargin * 2u);
				}
				Rectangle blendRect{ newPosition, 0, columnRect.Width, columnRect.Height };

				graphics.Blend(blendRect, draggingBox, { 0,0 }, 0.5);
			}
			graphics.DrawLine({ m_viewport.m_backgroundRect.X, (int)headerHeight - 1 }, { (int)m_window->ClientSize.Width - 1, (int)headerHeight - 1 }, m_appearance->BoxBorderColor);
			graphics.DrawLine({ headerOffset.X + headerWidthInt - 1, 0 }, { headerOffset.X + headerWidthInt - 1, (int)headerHeight - 1 }, m_appearance->BoxBorderColor);

			if (isSortedHeader)
			{
				int arrowWidth = m_window->ToScale(4);
				int arrowLength = m_window->ToScale(2);
				Rectangle arrowRect = columnRect;
				arrowRect.X += headerWidthInt - arrowSortedHeaderSize - sortedHeaderMargin;
				arrowRect.Width = arrowSortedHeaderSize;
				graphics.DrawArrow(arrowRect, arrowLength, arrowWidth, 
					m_headers.isAscendingOrdering ? Graphics::ArrowDirection::Upwards : Graphics::ArrowDirection::Downwards,
					m_appearance->Foreground2nd);
			}
			headerOffset.X += headerWidthInt;
		}
	}

	void ListBoxReactor::Module::DrawHeaderItem(Graphics& graphics, const Rectangle& rect, const std::string& name, bool isHovered, const Rectangle& textRect, const Color& textColor)
	{
		if (isHovered)
		{
			graphics.DrawRectangle(rect, m_appearance->HighlightColor, true);
		}

		DrawStringInBox(graphics, name, textRect, textColor);
	}

	void ListBoxReactor::Module::DrawList(Graphics& graphics)
	{
		bool enabled = true;
		auto headerHeight = m_window->ToScale(m_appearance->HeadersHeight);
		auto listItemIconSize = m_window->ToScale(m_appearance->ListItemIconSize);
		auto listItemIconMargin = m_window->ToScale(m_appearance->ListItemIconMargin);

		Point listOffset{ m_viewport.m_backgroundRect.X + (int)m_viewport.m_columnOffsetStartOff - m_scrollOffset.X, m_viewport.m_backgroundRect.Y - m_scrollOffset.Y };
		
		auto& itemHeight = m_viewport.m_itemHeight;
		auto& itemHeightWithMargin = m_viewport.m_itemHeightWithMargin;
		auto leftMarginListItemText = m_window->ToScale(3u);

		for (size_t i = m_viewport.m_startingVisibleIndex; i < m_viewport.m_endingVisibleIndex; i++)
		{
			auto absoluteIndex = m_list.m_sortedIndexes[i];
			auto& item = m_list.m_items[absoluteIndex];
			int cellOffset = 0;

			bool isLastSelected = &item == m_mouseSelection.m_selectedItem;
			bool isHovered = m_mouseSelection.m_hoveredItem == &item;
			bool isSelected = item.m_isSelected;
			Rectangle itemRect{ listOffset.X, listOffset.Y + (int)m_viewport.m_innerMargin + (int)(itemHeightWithMargin * i), m_viewport.m_contentSize.Width - m_viewport.m_columnOffsetStartOff, itemHeight };
			if (isSelected)
			{
				auto lineColor = enabled ? (isLastSelected ? m_appearance->Foreground2nd : (isSelected ? m_appearance->BoxBorderHighlightColor : m_appearance->BoxBorderColor)) : m_appearance->BoxBorderDisabledColor;

				auto color = m_appearance->HighlightColor;
				graphics.DrawRoundRectBox(itemRect, color, lineColor, true);
			}
			else if (isHovered)
			{
				auto color = m_appearance->ItemCollectionHightlightBackground;
				graphics.DrawRectangle(itemRect, color, true);
			}
			else if (isLastSelected)
			{
				auto color = m_appearance->Foreground2nd;
				graphics.DrawRectangle(itemRect, color, false);
			}

			for (size_t j = 0; j < item.m_cells.size(); j++)
			{
				const auto& headerIndex = m_headers.m_sorted[j];

				const auto& cell = item.m_cells[headerIndex];
				const auto& header = m_headers.m_items[headerIndex];
				auto headerWidth = m_window->ToScale(header.m_bounds.Width);
				uint32_t iconWidth = 0u;

				if (j == 0 && m_list.m_drawImages)
				{
					headerWidth += listItemIconSize + listItemIconMargin * 2u;
					iconWidth += listItemIconSize + listItemIconMargin * 2u;
				}
				auto headerWidthInt = static_cast<int>(headerWidth);
				if (listOffset.X + (int)leftMarginListItemText + cellOffset + headerWidthInt <= 0)
				{
					cellOffset += headerWidthInt;
					continue;
				}
				else if (cellOffset - m_scrollOffset.X >= (int)m_viewport.m_backgroundRect.Width)
				{
					break;
				}

				if (j == 0 && m_list.m_drawImages && item.m_icon)
				{
					auto iconSize = item.m_icon.GetSize();
					Rectangle destRect{ listOffset.X + (int)leftMarginListItemText + (int)listItemIconMargin, listOffset.Y + (int)(itemHeightWithMargin * i) + (int)(itemHeight- listItemIconSize) / 2 + (int)m_viewport.m_innerMargin, listItemIconSize,listItemIconSize};
					item.m_icon.Paste(iconSize.ToRectangle(), graphics, destRect);
				}

				DrawStringInBox(graphics, cell.m_text, { listOffset.X + (int)leftMarginListItemText + cellOffset + (int)iconWidth, listOffset.Y + (int)m_viewport.m_innerMargin + (int)(itemHeightWithMargin * i), headerWidth - leftMarginListItemText - iconWidth, itemHeight}, m_appearance->Foreground);

				cellOffset += headerWidthInt;
			}
		}
	}

	void ListBoxReactor::Module::CalculateSelectionBox(Point& startPoint, Point& endPoint, Size& boxSize) const
	{
		startPoint = {
			(std::min)(m_mouseSelection.m_startPosition.X, m_mouseSelection.m_endPosition.X),
			(std::min)(m_mouseSelection.m_startPosition.Y, m_mouseSelection.m_endPosition.Y)
		};

		endPoint = {
			(std::max)(m_mouseSelection.m_startPosition.X, m_mouseSelection.m_endPosition.X),
			(std::max)(m_mouseSelection.m_startPosition.Y, m_mouseSelection.m_endPosition.Y)
		};

		if (startPoint.Y < m_viewport.m_backgroundRect.Y - m_scrollOffset.Y)
		{
			startPoint.Y = m_viewport.m_backgroundRect.Y - m_scrollOffset.Y;
		}
		if (startPoint.X < m_viewport.m_backgroundRect.X - m_scrollOffset.X)
		{
			startPoint.X = m_viewport.m_backgroundRect.X - m_scrollOffset.X;
		}

		boxSize = { (uint32_t)(endPoint.X - startPoint.X), (uint32_t)(endPoint.Y - startPoint.Y) };
	}

	bool ListBoxReactor::Module::SetHoveredListItem(List::Item* index)
	{
		if (m_mouseSelection.m_hoveredItem != index)
		{
			m_mouseSelection.m_hoveredItem = index;
			return true;
		}
		return false;
	}

	void ListBoxReactor::Module::StopDragOrSortHeader()
	{
		if (m_headers.m_isDragging)
		{
			auto targetIndex = static_cast<size_t>(m_headers.m_draggingTargetIndex);
			auto selectedIndex = static_cast<size_t>(m_headers.m_selectedIndex);

			if (selectedIndex != targetIndex && (selectedIndex + 1) != targetIndex)
			{
				auto oldIndex = m_headers.m_sorted[selectedIndex];
				m_headers.m_sorted.emplace(m_headers.m_sorted.begin() + targetIndex, oldIndex);
				if (selectedIndex > targetIndex)
				{
					m_headers.m_sorted.erase(m_headers.m_sorted.begin() + selectedIndex + 1);
				}
				else
				{
					m_headers.m_sorted.erase(m_headers.m_sorted.begin() + selectedIndex);
				}
				BuildHeaderBounds();
			}
			m_headers.m_draggingBox.Release();
			m_headers.m_isDragging = false;
			m_headers.m_selectedIndex = -1;
		}
		else
		{
			bool ascending = m_headers.isAscendingOrdering;
			if (m_headers.m_sortedHeaderIndex != -1 && m_headers.m_sortedHeaderIndex == m_headers.m_sorted[m_headers.m_selectedIndex])
			{
				ascending = !ascending;
			}
			else
			{
				ascending = true;
			}
			size_t selectedHeaderIndex = static_cast<size_t>(m_headers.m_selectedIndex);

			SortHeader(m_headers.m_sorted[selectedHeaderIndex], ascending);
			m_headers.isAscendingOrdering = ascending;
			m_headers.m_sortedHeaderIndex = static_cast<int>(m_headers.m_sorted[selectedHeaderIndex]);
		}
		GUI::ReleaseCapture(m_window);
	}

	void ListBoxReactor::Module::SortHeader(size_t headerIndex, bool ascending)
	{
		if (m_list.m_items.empty() || headerIndex >= m_headers.m_items.size())
		{
			return;
		}

		m_list.m_sortedIndexes.resize(m_list.m_items.size());
		std::iota(m_list.m_sortedIndexes.begin(), m_list.m_sortedIndexes.end(), 0);

		auto compare = [this, headerIndex, ascending](std::size_t idxA, std::size_t idxB)
			{
				const auto& textA = m_list.m_items[idxA].m_cells[headerIndex].m_text;
				const auto& textB = m_list.m_items[idxB].m_cells[headerIndex].m_text;
				return ascending ? textA < textB : textA > textB;
			};

		std::sort(m_list.m_sortedIndexes.begin(), m_list.m_sortedIndexes.end(), compare);
	}

	int ListBoxReactor::Module::GetListItemIndex(List::Item* item)
	{
		for (size_t i = 0; i < m_list.m_items.size(); i++)
		{
			if (item == &m_list.m_items[m_list.m_sortedIndexes[i]])
			{
				return static_cast<int>(i);
			}
		}
		return -1;
	}

	bool ListBoxReactor::Module::HandleMultiSelection(List::Item* item, const ArgMouse& args)
	{
		bool needUpdate = false;
		auto absoluteIndex = GetListItemIndex(item);
		if (!m_mouseSelection.m_pivotItem)
		{
			m_mouseSelection.m_pivotItem = item;
		}

		if (!m_ctrlPressed && !m_shiftPressed)
		{
			if (!args.ButtonState.RightButton || !item->m_isSelected)
			{
				ClearSelection();
			}
			if (!item->m_isSelected)
			{
				SelectItem(item);
				needUpdate = true;
			}
		}
		else if (m_shiftPressed && m_mouseSelection.m_selectedItem)
		{
			ClearSelection();
			PerformRangeSelection(item);
			needUpdate = true;
		}
		else
		{
			ToggleItemSelection(item);
			needUpdate = true;
		}
		needUpdate |= m_mouseSelection.m_selectedItem != item;
		m_mouseSelection.m_selectedItem = item;

		needUpdate |= EnsureVisibility(absoluteIndex);
		return needUpdate;
	}

	bool ListBoxReactor::Module::UpdateSingleSelection(List::Item* item)
	{
		bool needUpdate = m_mouseSelection.m_selectedItem != item || !item->m_isSelected;
		if (needUpdate)
		{
			ClearSingleSelection();
			SelectItem(item);
		}
		return needUpdate;
	}

	void ListBoxReactor::Module::ToggleItemSelection(List::Item* item)
	{
		item->m_isSelected = !item->m_isSelected;

		if (item->m_isSelected)
		{
			m_mouseSelection.m_selections.push_back(item);
		}
		else
		{
			auto it = std::remove(m_mouseSelection.m_selections.begin(), m_mouseSelection.m_selections.end(), item);
			m_mouseSelection.m_selections.erase(it, m_mouseSelection.m_selections.end());
		}
	}

	void ListBoxReactor::Module::StartSelectionRectangle(const Point& mousePosition)
	{
		auto logicalPosition = mousePosition;
		logicalPosition -= m_scrollOffset;
		m_mouseSelection.m_started = true;
		m_mouseSelection.m_startPosition = logicalPosition;
		m_mouseSelection.m_endPosition = logicalPosition;

		m_mouseSelection.m_inverseSelection = (m_ctrlPressed && !m_shiftPressed);

		m_mouseSelection.m_selections.clear();
		m_mouseSelection.m_alreadySelected.clear();

		for (size_t i = 0; i < m_list.m_items.size(); i++)
		{
			if (m_list.m_items[i].m_isSelected)
			{
				m_mouseSelection.m_selections.push_back(&m_list.m_items[i]);
				m_mouseSelection.m_alreadySelected.push_back(&m_list.m_items[i]);
			}
		}

		GUI::Capture(m_window);
	}

	bool ListBoxReactor::Module::ClearSelectionIfNeeded()
	{
		if (!m_ctrlPressed && !m_shiftPressed)
		{
			if (!m_mouseSelection.m_selections.empty())
			{
				for (const auto& item : m_mouseSelection.m_selections)
				{
					item->m_isSelected = false;
				}
				m_mouseSelection.m_selections.clear();
				m_mouseSelection.m_alreadySelected.clear();

				//m_mouseSelection.m_selectedItem = nullptr;
				m_mouseSelection.m_pivotItem = nullptr;
				return true;
			}
		}
		return false;
	}

	bool ListBoxReactor::Module::ClearSingleSelection()
	{
		if (m_mouseSelection.m_selectedItem)
		{
			m_mouseSelection.m_selectedItem->m_isSelected = false;
			m_mouseSelection.m_selections.clear();
			m_mouseSelection.m_selectedItem = nullptr;
			return true;
		}
		return false;
	}

	std::vector<ListBoxItem> ListBoxReactor::Module::GetSelectedItems()
	{
		std::vector<ListBoxItem> selections;
		std::vector<size_t> indexes;

		selections.reserve(m_mouseSelection.m_selections.size());
		indexes.reserve(m_mouseSelection.m_selections.size());

		for (size_t i = 0; i < m_mouseSelection.m_selections.size(); i++)
		{
			auto index = GetListItemIndex(m_mouseSelection.m_selections[i]);
			indexes.emplace_back(index);
		}
		std::sort(indexes.begin(), indexes.end());
		for (size_t i = 0; i < m_mouseSelection.m_selections.size(); i++)
		{
			selections.emplace_back(&m_list.m_items[m_list.m_sortedIndexes[indexes[i]]], this);
		}
		return selections;
	}

	void ListBoxReactor::Module::ClearSelection()
	{
		for (size_t i = 0; i < m_mouseSelection.m_selections.size(); i++)
		{
			m_mouseSelection.m_selections[i]->m_isSelected = false;
		}
		m_mouseSelection.m_selections.clear();
	}

	void ListBoxReactor::Module::SelectItem(List::Item* item)
	{
		item->m_isSelected = true;
		m_mouseSelection.m_selections.push_back(item);
		m_mouseSelection.m_selectedItem = item;
	}

	bool ListBoxReactor::Module::EnsureVisibility(int lastLocalSelectedIndex)
	{
		if (!m_scrollBarVert)
		{
			return false;
		}

		Rectangle itemBounds{ m_viewport.m_backgroundRect.X, - m_scrollOffset.Y + (int)m_viewport.m_innerMargin + (int)(m_viewport.m_itemHeightWithMargin * lastLocalSelectedIndex),
			m_viewport.m_backgroundRect.Width, 
			m_viewport.m_itemHeight
		};

		if (itemBounds.Y >= 0 && itemBounds.Y + (int)itemBounds.Height <= (int)m_viewport.m_backgroundRect.Height)
		{
			return false;
		}

		auto offsetAdjustment = 0;
		if (itemBounds.Y + (int)itemBounds.Height >= (int)m_viewport.m_backgroundRect.Height)
		{
			offsetAdjustment = itemBounds.Y + (int)(itemBounds.Height - m_viewport.m_backgroundRect.Height + m_viewport.m_innerMargin);
		}
		else
		{
			offsetAdjustment = itemBounds.Y - (int)m_viewport.m_innerMargin;
		}
		m_scrollOffset.Y = std::clamp(m_scrollOffset.Y + offsetAdjustment, m_scrollBarVert->GetMin(), m_scrollBarVert->GetMax());
		CalculateVisibleIndices();

		m_scrollBarVert->SetValue(m_scrollOffset.Y);

		GUI::UpdateWindow(m_scrollBarVert->Handle());
		return true;
	}

	void ListBoxReactor::Module::PerformRangeSelection(List::Item* pressedItem)
	{
		auto pressedItemIndex = GetListItemIndex(pressedItem);
		auto pivotIndex = GetListItemIndex(m_mouseSelection.m_pivotItem);
		int minIndex = (std::min)(pivotIndex, pressedItemIndex);
		int maxIndex = (std::max)(pivotIndex, pressedItemIndex);

		for (int i = minIndex; i <= maxIndex; ++i)
		{
			auto currentItem = &m_list.m_items[m_list.m_sortedIndexes[i]];
			currentItem->m_isSelected = true;
			m_mouseSelection.m_selections.push_back(currentItem);
		}
	}

	Berta::ListBoxReactor::InteractionArea ListBoxReactor::Module::DetermineHoverArea(const Point& mousePosition)
	{
		auto headerHeight = m_window->ToScale(m_appearance->HeadersHeight);
		if (!m_window->ClientSize.IsInside(mousePosition))
		{
			return InteractionArea::None;
		}

		if (mousePosition.Y <= (int)headerHeight)
		{
			Point headerOffset{ m_viewport.m_backgroundRect.X + (int)m_viewport.m_columnOffsetStartOff - m_scrollOffset.X, 0 };

			auto listItemIconSize = m_window->ToScale(m_appearance->ListItemIconSize);
			auto listItemIconMargin = m_window->ToScale(m_appearance->ListItemIconMargin);

			auto splitterThreshold = m_window->ToScale(3);
			for (size_t i = 0; i < m_headers.m_items.size(); i++)
			{
				const auto& headerIndex = m_headers.m_sorted[i];
				const auto& header = m_headers.m_items[headerIndex];
				auto headerWidth = m_window->ToScale(header.m_bounds.Width);
				if (i == 0 && m_list.m_drawImages)
				{
					headerWidth += listItemIconSize + listItemIconMargin * 2u;
				}
				auto headerWidthInt = (int)headerWidth;
				if (headerOffset.X + headerWidthInt < -splitterThreshold || headerOffset.X - splitterThreshold >= (int)m_viewport.m_backgroundRect.Width)
				{
					headerOffset.X += headerWidthInt;
					continue;
				}

				if (mousePosition.X >= headerOffset.X + headerWidthInt - splitterThreshold &&
					mousePosition.X <= headerOffset.X + headerWidthInt + splitterThreshold)
				{
					return InteractionArea::HeaderSplitter;
				}
				else if (mousePosition.X >= headerOffset.X &&
					mousePosition.X < headerOffset.X + headerWidthInt - splitterThreshold)
				{
					return InteractionArea::Header;
				}

				headerOffset.X += headerWidthInt;
			}

			return InteractionArea::None;
		}

		if (m_viewport.m_needVerticalScroll && m_viewport.m_needHorizontalScroll && 
			mousePosition.X >= (int)(m_viewport.m_backgroundRect.Width) &&
			mousePosition.Y >= (int)(m_viewport.m_backgroundRect.Height))
		{
			return InteractionArea::None;
		}

		auto itemHeight = m_window->ToScale(m_appearance->ListItemHeight) + m_viewport.m_innerMargin * 2u;
		auto itemHeightInt = static_cast<int>(itemHeight);

		auto positionX = mousePosition.X - m_viewport.m_backgroundRect.X + m_scrollOffset.X;
		auto positionY = mousePosition.Y - m_viewport.m_backgroundRect.Y + m_scrollOffset.Y;
		int index = positionY / itemHeightInt;

		if (index < m_list.m_items.size())
		{
			if (m_viewport.m_innerMargin)
			{
				auto topBound = index * itemHeightInt;
				auto bottomBound = topBound + itemHeightInt;
				if ((positionY >= topBound && positionY <= topBound + (int)m_viewport.m_innerMargin) ||
					(positionY >= bottomBound - (int)m_viewport.m_innerMargin && positionY <= bottomBound) || positionX > (int)m_viewport.m_contentSize.Width)
				{
					return InteractionArea::ListBlank;
				}
			}

			return InteractionArea::List;
		}

		return InteractionArea::ListBlank;
	}

	bool ListBoxReactor::MouseSelection::IsAlreadySelected(List::Item* index) const
	{
		return std::find(m_alreadySelected.begin(), m_alreadySelected.end(), index) != m_alreadySelected.end();
	}

	bool ListBoxReactor::MouseSelection::IsSelected(List::Item* item) const
	{
		return std::find(m_selections.begin(), m_selections.end(), item) != m_selections.end();
	}

	void ListBoxReactor::MouseSelection::Select(List::Item* item)
	{
		m_selections.push_back(item);
	}

	void ListBoxReactor::MouseSelection::Deselect(List::Item* item)
	{
		auto it = std::find(m_selections.begin(), m_selections.end(), item);
		if (it != m_selections.end())
		{
			m_selections.erase(it);
		}
	}

	void ListBoxReactor::MouseSelection::Clear()
	{
		m_selections.clear();
		m_alreadySelected.clear();

		m_selectedItem = nullptr;
		m_pivotItem = nullptr;
		m_hoveredItem = nullptr;
	}

	void ListBoxReactor::MouseSelection::ClearReferences(List::Item* item)
	{
		if (m_selectedItem == item)
		{
			m_selectedItem = nullptr;
		}
		if (m_pivotItem == item)
		{
			m_pivotItem = nullptr;
		}
		if (m_hoveredItem == item)
		{
			m_hoveredItem = nullptr;
		}
	}

	int ListBoxReactor::Module::GetHeaderAtMousePosition(const Point& mousePosition, bool splitter)
	{
		auto listItemIconSize = m_window->ToScale(m_appearance->ListItemIconSize);
		auto listItemIconMargin = m_window->ToScale(m_appearance->ListItemIconMargin);
		Point headerOffset{ m_viewport.m_backgroundRect.X + (int)m_viewport.m_columnOffsetStartOff - m_scrollOffset.X, 0 };
		
		auto splitterThreshold = m_window->ToScale(3);
		for (size_t i = 0; i < m_headers.m_items.size(); i++)
		{
			const auto& headerIndex = m_headers.m_sorted[i];
			const auto& header = m_headers.m_items[headerIndex];
			auto headerWidth = m_window->ToScale(header.m_bounds.Width);
			if (i == 0 && m_list.m_drawImages)
			{
				headerWidth += listItemIconSize + listItemIconMargin * 2u;
			}
			auto headerWidthInt = (int)headerWidth;
			if (headerOffset.X + headerWidthInt < -splitterThreshold || headerOffset.X - splitterThreshold >= (int)m_viewport.m_backgroundRect.Width)
			{
				headerOffset.X += headerWidthInt;
				continue;
			}

			if (splitter && mousePosition.X >= headerOffset.X + headerWidthInt - splitterThreshold &&
				mousePosition.X <= headerOffset.X + headerWidthInt + splitterThreshold)
			{
				return (int)i;
			}
			else if (!splitter && mousePosition.X >= headerOffset.X &&
				mousePosition.X < headerOffset.X + headerWidthInt)
			{
				return (int)i;
			}

			headerOffset.X += headerWidthInt;
		}
		return -1;
	}

	void ListBoxReactor::Module::StartHeadersSizing(const Point& mousePosition)
	{
		GUI::Capture(m_window);
		m_headers.m_selectedIndex = GetHeaderAtMousePosition(mousePosition, true);
		int iconWidth = 0;
		if (m_list.m_drawImages && m_headers.m_selectedIndex == 0)
		{
			auto listItemIconSize = m_window->ToScale(m_appearance->ListItemIconSize);
			auto listItemIconMargin = m_window->ToScale(m_appearance->ListItemIconMargin);
			iconWidth += listItemIconSize + listItemIconMargin * 2u;
		}
		const auto& headerIndex = m_headers.m_sorted[m_headers.m_selectedIndex];
		m_headers.m_mouseDownOffset = m_scrollOffset.X + mousePosition.X - (int)m_window->ToScale(m_headers.m_items[headerIndex].m_bounds.X + m_headers.m_items[headerIndex].m_bounds.Width + iconWidth);
		
	}

	void ListBoxReactor::Module::UpdateHeadersSize(const Point& mousePosition)
	{
		const auto& headerIndex = m_headers.m_sorted[m_headers.m_selectedIndex];
		auto& headerBounds = m_headers.m_items[headerIndex].m_bounds;
		auto newWidth = m_window->ToDownwardScale(m_scrollOffset.X + mousePosition.X - m_headers.m_mouseDownOffset - m_window->ToScale(headerBounds.X));
		int iconWidth = 0;
		if (m_list.m_drawImages && m_headers.m_selectedIndex == 0)
		{
			auto listItemIconSize = m_window->ToScale(m_appearance->ListItemIconSize);
			auto listItemIconMargin = m_window->ToScale(m_appearance->ListItemIconMargin);
			iconWidth += listItemIconSize + listItemIconMargin * 2u;
			newWidth -= iconWidth;
		}
		headerBounds.Width = (std::max)(LISTBOX_MIN_HEADER_WIDTH, static_cast<uint32_t>((std::max)(0, newWidth)));
		CalculateViewport(m_viewport);
		CalculateVisibleIndices();
		
		if (UpdateScrollBars())
		{
			if (m_scrollBarVert)
				m_scrollBarVert->Handle()->Renderer.Update();

			if (m_scrollBarHoriz)
				m_scrollBarHoriz->Handle()->Renderer.Update();
		}
		BuildHeaderBounds(m_headers.m_selectedIndex);
	}

	void ListBoxReactor::Module::StopHeadersSizing()
	{
		GUI::ReleaseCapture(m_window);
		m_headers.m_selectedIndex = -1;
	}

	void ListBoxReactor::Module::StartSelectingHeader(const Point& mousePosition)
	{
		GUI::Capture(m_window);
		m_headers.m_isDragging = false;
		m_headers.m_selectedIndex = GetHeaderAtMousePosition(mousePosition, false);
		const auto& headerIndex = m_headers.m_sorted[m_headers.m_selectedIndex];
		m_headers.m_mouseDownOffset = mousePosition.X - m_window->ToScale(m_headers.m_items[headerIndex].m_bounds.X) - m_viewport.m_backgroundRect.X - (int)m_viewport.m_columnOffsetStartOff + m_scrollOffset.X;
	}

	void ListBoxItem::SetIcon(const Image& image)
	{
		m_target->m_icon = image;
		if (image)
		{
			m_module->m_list.m_drawImages = true;
			m_module->BuildHeaderBounds();
		}
	}

	void ListBoxItem::SetText(size_t columnIndex, const std::string& text)
	{
		if (columnIndex >= m_module->m_headers.m_items.size())
			return;

		bool needUpdate = m_target->m_cells[columnIndex].m_text != text;

		m_target->m_cells[columnIndex].m_text = text;
		if (m_module->m_headers.m_sortedHeaderIndex == columnIndex)
		{
			m_module->SortHeader(m_module->m_headers.m_sorted[m_module->m_headers.m_sortedHeaderIndex], m_module->m_headers.isAscendingOrdering);
		}

		if (needUpdate)
		{
			GUI::UpdateWindow(m_module->m_window);
		}
	}

	std::string ListBoxItem::GetText(size_t columnIndex)
	{
		if (columnIndex >= m_module->m_headers.m_items.size())
			return std::string();

		return m_target->m_cells[columnIndex].m_text;
	}

	ListBox::ListBox(Window* parent, const Rectangle& rectangle)
	{
		Create(parent, true, rectangle);

#if BT_DEBUG
		m_handle->Name = "ListBox";
#endif
	}

	void ListBox::AppendHeader(const std::string& name, uint32_t width)
	{
		m_reactor.GetModule().AppendHeader(name, width);
	}

	ListBoxItem ListBox::Append(const std::string& text)
	{
		return m_reactor.GetModule().Append(text);
	}

	ListBoxItem ListBox::Append(std::initializer_list<std::string> texts)
	{
		return m_reactor.GetModule().Append(texts);
	}

	ListBoxItem ListBox::At(size_t index)
	{
		return m_reactor.GetModule().At(index);
	}

	void ListBox::Clear()
	{
		m_reactor.GetModule().Clear();
	}

	void ListBox::ClearHeaders()
	{
		m_reactor.GetModule().ClearHeaders();
	}

	void ListBox::Erase(ListBoxItem item)
	{
		m_reactor.GetModule().Erase(item);
	}

	void ListBox::Erase(std::vector<ListBoxItem>& items)
	{
		m_reactor.GetModule().Erase(items);
	}

	void ListBox::EnableMultiselection(bool enabled)
	{
		m_reactor.GetModule().EnableMultiselection(enabled);
	}

	std::vector<ListBoxItem> ListBox::GetSelected()
	{
		return m_reactor.GetModule().GetSelectedItems();
	}
}
