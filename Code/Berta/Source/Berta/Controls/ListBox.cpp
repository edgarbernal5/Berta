/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#include "btpch.h"
#include "ListBox.h"

#include "Berta/GUI/Interface.h"
#include "Berta/GUI/EnumTypes.h"

#include <algorithm>
#include <numeric>

#include "Berta/GUI/ScopedClip.h"

namespace Berta
{
	namespace Internal::ListBox
	{
		size_t ItemCollection::Append(const std::string& text)
		{
			List::Item newItem;
			newItem.m_cells.emplace_back(text);
        
			m_items.emplace_back(std::move(newItem));
			size_t newIndex = m_items.size() - 1;
        
			m_visualMap.emplace_back(newIndex);
			
			TriggerChanged(std::nullopt);
			return newIndex;
		}

		size_t ItemCollection::Append(std::initializer_list<std::string> texts)
		{
			List::Item newItem;
			for (const auto& text : texts)
			{
				newItem.m_cells.emplace_back(text);
			}
        
			m_items.emplace_back(std::move(newItem));
			size_t newIndex = m_items.size() - 1;
			m_visualMap.emplace_back(newIndex);
        
			TriggerChanged(std::nullopt);
			return newIndex;
		}

		void ItemCollection::Clear()
		{
			if (!m_items.empty())
			{
				m_items.clear();
				m_visualMap.clear();
				TriggerChanged(std::nullopt);
			}
		}

		void ItemCollection::RemoveAt(size_t index)
		{
			if (index < m_items.size())
			{
				m_items.erase(m_items.begin() + index);
				m_visualMap.erase(m_visualMap.begin() + index);
				TriggerChanged(std::nullopt);
			}
		}

		void ItemCollection::Erase(size_t logicalIndex)
		{
			if (logicalIndex >= m_items.size())
			{
				return;
			}
			
			m_items.erase(m_items.begin() + logicalIndex);

			auto it = std::find(m_visualMap.begin(), m_visualMap.end(), logicalIndex);
			if (it != m_visualMap.end())
			{
				m_visualMap.erase(it);
			}

			for (size_t& mappedLogicalIndex : m_visualMap)
			{
				if (mappedLogicalIndex > logicalIndex)
				{
					mappedLogicalIndex--;
				}
			}

			TriggerChanged(std::nullopt);
		}

		List::Item* ItemCollection::At(size_t index)
		{
			if (index < m_items.size())
			{
				return &m_items[index];
			}
			return nullptr;
		}

		void ItemCollection::Sort(size_t columnIndex, bool ascending)
		{
			if (m_items.empty())
			{
				return;
			}
			std::sort(m_visualMap.begin(), m_visualMap.end(), [&](size_t logicalA, size_t logicalB) 
			{
				const auto& itemA = m_items[logicalA];
				const auto& itemB = m_items[logicalB];

				std::string textA = (columnIndex < itemA.m_cells.size()) ? itemA.m_cells[columnIndex].m_text : "";
				std::string textB = (columnIndex < itemB.m_cells.size()) ? itemB.m_cells[columnIndex].m_text : "";

				return ascending ? (textA < textB) : (textA > textB);
			});
			TriggerChanged(std::nullopt);
		}

		void ItemCollection::ResetSort()
		{
			for (size_t i = 0; i < m_visualMap.size(); ++i)
			{
				m_visualMap[i] = i;
			}
			TriggerChanged(std::nullopt);
		}

		void ItemCollection::EnableImages(bool active)
		{
			m_drawImages = active;
		}

		size_t ItemCollection::GetLogicalIndex(size_t visualIndex) const
		{
			if (visualIndex < m_visualMap.size())
			{
				return m_visualMap[visualIndex];
			}
			return static_cast<size_t>(-1);
		}

		size_t ItemCollection::GetVisualIndex(size_t logicalIndex) const
		{
			auto it = std::find(m_visualMap.begin(), m_visualMap.end(), logicalIndex);
			if (it != m_visualMap.end())
			{
				return static_cast<size_t>(std::distance(m_visualMap.begin(), it));
			}
    
			return static_cast<size_t>(-1);
		}

		void ItemCollection::NotifyItemModified(std::optional<size_t> logicalIndex)
		{
			TriggerChanged(logicalIndex);
		}

		void ItemCollection::TriggerChanged(std::optional<size_t> logicalIndex)
		{
			if (m_onChanged)
			{
				m_onChanged(logicalIndex);
			}
		}

		void Reactor::DoOnInit()
		{
			m_module.m_window = m_control->Handle();
			m_module.m_control = m_control;

			GUI::SetWindowBorderless(*m_control, false);
			
			m_module.m_headers.Init(m_module.m_window);
			m_module.m_items.SetOnItemChangedCallback([this](std::optional<size_t> logicalIndex)
			{
				if (logicalIndex.has_value())
				{
					size_t visualIndex = m_module.m_items.GetVisualIndex(*logicalIndex);
					if (visualIndex != static_cast<size_t>(-1))
					{
						Rectangle itemRect = m_module.GetItemLocalRect(visualIndex);
						GUI::MarkAsNeedUpdate(m_module.m_window, &itemRect);
						return;
					}
				}

				// Si no hay índice (std::nullopt) o el ítem ya no es visible, 
				// hacemos el fallback seguro: recalculamos scroll e invalidamos TODO el control.
				m_module.UpdateScrollData();
				
				GUI::MarkAsNeedUpdate(m_module.m_window);
			});
			
			m_module.m_headers.SetOnHeaderClickedCallback([this](size_t visualColumnIndex)
			{
				if (m_module.m_currentSortColumn.has_value() && m_module.m_currentSortColumn == visualColumnIndex)
				{
					m_module.m_isSortAscending = !m_module.m_isSortAscending;
				}
				else
				{
					m_module.m_currentSortColumn = visualColumnIndex;
					m_module.m_isSortAscending = true;
				}
				m_module.m_headers.SetSortState(m_module.m_currentSortColumn.value(), m_module.m_isSortAscending);
				m_module.m_items.Sort(visualColumnIndex, m_module.m_isSortAscending);
			});
			
			m_module.m_headers.SetOnRequestColumnAutoWidth([this](size_t logicalColumnIndex) -> uint32_t
			{
				const auto& headers = m_module.m_headers.GetHeaders();
				std::string headerText = headers[logicalColumnIndex].Text;
				uint32_t maxWidth = m_graphics->GetTextExtent(headerText).Width;

				auto appearance = reinterpret_cast<Appearance*>(m_module.m_window->Appearance.get());
				auto listItemIconSize = m_module.m_window->ToScale(appearance->ListItemIconSize);
				auto listItemIconMargin = m_module.m_window->ToScale(appearance->ListItemIconMargin);
				
				bool showIcons = m_module.m_headers.GetLogicalIndex(0) == logicalColumnIndex && m_module.m_items.ShouldDrawImages();
				size_t rowCount = m_module.m_items.GetCount();
				for (size_t i = 0; i < rowCount; ++i)
				{
					auto* item = m_module.m_items.GetItemSafely(i);
		            
					if (item && logicalColumnIndex < item->m_cells.size())
					{
						std::string cellText = item->m_cells[logicalColumnIndex].m_text; 
		                
						auto textWidth = m_graphics->GetTextExtent(cellText).Width;
						if (showIcons)
						{
							textWidth += listItemIconSize + 2u * listItemIconMargin;
						}
						maxWidth = std::max<uint32_t>(textWidth, maxWidth);
					}
				}

				return maxWidth;
			});
			
			m_module.InitScrollableView();
			m_module.UpdateScrollData();
		}

		void Reactor::Update(Graphics& graphics)
		{
			auto appearance = reinterpret_cast<Appearance*>(m_module.m_window->Appearance.get());
			auto globalRect = m_module.m_window->ClientSize.ToRectangle();
			graphics.FillRectangle(globalRect, appearance->BoxBackground);
			
			m_module.m_headers.Draw(graphics, m_module.m_scrollableView->GetVisibleRect(), m_module.m_scrollableView->GetClientArea());
			m_module.DrawList(graphics);
			
			if (m_module.m_lassoSelection.IsActive())
			{
				Graphics selectionBox(m_module.m_lassoSelection.GetRect(), m_module.m_window->DPI, m_module.m_window->RootPaintHandle);
				m_module.m_lassoSelection.Draw(graphics, selectionBox, appearance->SelectionHighlightColor, appearance->SelectionBorderHighlightColor);
			}
			
			if (m_module.m_scrollableView->HasHorizontalScroll() && m_module.m_scrollableView->HasVerticalScroll())
			{
				auto scrollSize = m_module.m_window->ToScale(appearance->ScrollBarSize);
				graphics.FillRectangle
				(
					{
						static_cast<int>(globalRect.Width) - static_cast<int>(scrollSize) - 1, 
						static_cast<int>(globalRect.Height) - static_cast<int>(scrollSize) - 1,
						scrollSize,
						scrollSize
					},
					appearance->Background
				);
			}
		}

		void Reactor::DblClick(Graphics& graphics, const ArgMouse& args)
		{
			int scrollX = m_module.m_scrollableView->GetVisibleRect().X;
			if (m_module.m_headers.OnDblClick(args, scrollX))
			{
				m_module.UpdateScrollData();
				m_module.m_headers.OnMouseMove(args, scrollX);
				
				GUI::MarkAsNeedUpdate(m_module.m_window);
				return;
			}
		}

		void Reactor::Resize(Graphics& graphics, const ArgResize& args)
		{
			m_module.UpdateScrollData();
		}

		void Reactor::MouseDown(Graphics& graphics, const ArgMouse& args)
		{
			auto scrollOffset = m_module.m_scrollableView->GetVisibleRect();

			if (m_module.m_headers.OnMouseDown(args, scrollOffset.X))
			{
				return;
			}

			auto appearance = reinterpret_cast<Appearance*>(m_module.m_window->Appearance.get());
			int headerHeight = static_cast<int>(m_module.m_window->ToScale(appearance->HeadersHeight));
			auto innerMargin = m_module.m_window->ToScale(2u);
			int itemHeight = static_cast<int>(m_module.m_window->ToScale(appearance->ListItemHeight));
			int itemHeightWithMargin = static_cast<int>(itemHeight + innerMargin * 2u);
			int absoluteY = args.Position.Y - headerHeight + scrollOffset.Y;
			
			size_t clickedVisualIndex = absoluteY / itemHeightWithMargin;
			if (clickedVisualIndex < m_module.m_items.GetCount())
			{
				size_t clickedLogicalIndex = m_module.m_items.GetLogicalIndex(clickedVisualIndex);
				bool isCtrl = args.CtrlPressed;
				bool isShift = args.ShiftPressed;

				auto rangeResolver = [&](size_t anchorLogical, size_t currentLogical)
				{
					std::vector<size_t> result;
            
					size_t anchorVisual = m_module.m_items.GetVisualIndex(anchorLogical);
					size_t currentVisual = clickedVisualIndex;

					size_t start = std::min<size_t>(anchorVisual, currentVisual);
					size_t end = std::max<size_t>(anchorVisual, currentVisual);
            
					for (size_t i = start; i <= end; ++i)
					{
						result.emplace_back(m_module.m_items.GetLogicalIndex(i));
					}
					return result;
				};
				
				m_module.m_focusedLogicalIndex= clickedLogicalIndex;
				m_module.EnsureVisible(clickedVisualIndex);
				if (m_module.m_selectionController.Select(clickedLogicalIndex, isCtrl, isShift, rangeResolver))
				{
					GUI::MarkAsNeedUpdate(m_module.m_window);
					m_module.TriggerSelectionChanged();
				}
			}
			else
			{
				if (!args.CtrlPressed && !args.ShiftPressed)
				{
					m_module.m_selectionController.Clear();
					GUI::MarkAsNeedUpdate(m_module.m_window);
				}
				
				if (m_module.m_selectionController.IsMultiSelect())
				{
					m_module.m_selectionController.SaveSnapshot();
					m_module.m_lassoSelection.Start(args.Position);
					GUI::Capture(m_module.m_window);
				}
			}
		}

		void Reactor::MouseMove(Graphics& graphics, const ArgMouse& args)
		{
			auto scrollOffset = m_module.m_scrollableView->GetVisibleRect();
			if (m_module.m_headers.OnMouseMove(args, scrollOffset.X))
			{
				if (m_module.m_headers.IsResizing())
				{
					m_module.UpdateScrollData();
				}
				
				if (m_module.m_hoveredIndex.has_value())
				{
					Rectangle oldRect = m_module.GetItemLocalRect(*m_module.m_hoveredIndex);
					GUI::MarkAsNeedUpdate(m_module.m_window, &oldRect);
					
					m_module.m_hoveredIndex.reset();
				}
				return;
			}
			
			m_module.m_headers.OnMouseLeave();
			if (m_module.m_lassoSelection.IsActive())
			{
				if (m_module.m_lassoSelection.Update(args.Position))
				{
					m_module.ProcessLassoIntersection(args.CtrlPressed);
					GUI::MarkAsNeedUpdate(m_module.m_window);
				}
				return;
			}
			
			auto appearance = reinterpret_cast<Appearance*>(m_module.m_window->Appearance.get());
			int headerHeight = static_cast<int>(m_module.m_window->ToScale(appearance->HeadersHeight));

			if (args.Position.Y < headerHeight)
			{
				if (m_module.m_hoveredIndex.has_value())
				{
					Rectangle newRect = m_module.GetItemLocalRect(*m_module.m_hoveredIndex);
					GUI::MarkAsNeedUpdate(m_module.m_window, &newRect);
					
					m_module.m_hoveredIndex.reset();
				}
				return;
			}
			auto innerMargin = m_module.m_window->ToScale(2u);
			int itemHeight = static_cast<int>(m_module.m_window->ToScale(appearance->ListItemHeight));
			int itemHeightWithMargin = static_cast<int>(itemHeight + innerMargin * 2u);
			int absoluteY = args.Position.Y - headerHeight + scrollOffset.Y;
			int hoveredIndex = absoluteY / itemHeightWithMargin;

			if (hoveredIndex >= 0 && hoveredIndex < static_cast<int>(m_module.m_items.GetCount()))
			{
				if (m_module.m_hoveredIndex != hoveredIndex)
				{
					if (m_module.m_hoveredIndex.has_value())
					{
						Rectangle oldRect = m_module.GetItemLocalRect(*m_module.m_hoveredIndex);
						GUI::MarkAsNeedUpdate(m_module.m_window, &oldRect);
					}
					m_module.m_hoveredIndex = hoveredIndex;
					
					Rectangle newRect = m_module.GetItemLocalRect(hoveredIndex);
					GUI::MarkAsNeedUpdate(m_module.m_window, &newRect);
				}
			}
			else if (m_module.m_hoveredIndex.has_value())
			{
				Rectangle newRect = m_module.GetItemLocalRect(*m_module.m_hoveredIndex);
				GUI::MarkAsNeedUpdate(m_module.m_window, &newRect);
				
				m_module.m_hoveredIndex.reset();
			}
		}

		void Reactor::MouseUp(Graphics& graphics, const ArgMouse& args)
		{
			if (m_module.m_headers.OnMouseUp(args, m_module.m_scrollableView->GetVisibleRect().X))
			{
				return;
			}

			if (m_module.m_lassoSelection.IsActive())
			{
				bool actuallyChanged = m_module.m_selectionController.HasSelectionChangedSinceSnapshot();
				
				m_module.m_lassoSelection.End();
				m_module.m_selectionController.ClearSnapshot();
				
				GUI::ReleaseCapture(m_module.m_window);
				GUI::MarkAsNeedUpdate(m_module.m_window);
				
				if (actuallyChanged)
				{
					m_module.TriggerSelectionChanged();
				}
			}
		}

		void Reactor::MouseLeave(Graphics& graphics, const ArgMouse& args)
		{
			m_module.m_headers.OnMouseLeave();

			if (m_module.m_hoveredIndex.has_value())
			{
				m_module.m_hoveredIndex.reset();
				GUI::MarkAsNeedUpdate(m_module.m_window);
			}
		}

		void Reactor::MouseWheel(Graphics& graphics, const ArgWheel& args)
		{
			if (m_module.m_scrollableView)
			{
				m_module.m_scrollableView->HandleMouseWheel(args);
			}
		}

		void Reactor::KeyPressed(Graphics& graphics, const ArgKeyboard& args)
		{
			size_t itemCount = m_module.m_items.GetCount();
			if (itemCount == 0)
			{
				return;
			}
    
			// Guardamos el índice visual anterior antes de cualquier operación
			std::optional<size_t> prevVisual;
			if (m_module.m_focusedLogicalIndex.has_value())
			{
				prevVisual = m_module.m_items.GetVisualIndex(m_module.m_focusedLogicalIndex.value());
			}
    
			size_t currentVisual = (prevVisual.has_value()) ? *prevVisual : 0;
			bool focusChanged = false;
			bool selectionChanged = false;
    
			if (args.Key == KeyboardKey::ArrowDown)
			{
				if (currentVisual + 1 < itemCount)
				{
					currentVisual++;
					focusChanged = true;
				}
			}
			else if (args.Key == KeyboardKey::ArrowUp)
			{
				if (currentVisual > 0)
				{
					currentVisual--;
					focusChanged = true;
				}
			}
			else if (args.Key == KeyboardKey::Space)
			{
				size_t logicalToSelect = m_module.m_items.GetLogicalIndex(currentVisual);
				selectionChanged = m_module.SelectItemConResolver(logicalToSelect, args.ButtonState.Ctrl, args.ButtonState.Shift);
       
				m_module.m_focusedLogicalIndex = logicalToSelect;
       
				// Barra espaciadora: solo cambia la selección de la fila actual
				if (selectionChanged)
				{
					Rectangle rect = m_module.GetItemLocalRect(currentVisual);
					GUI::MarkAsNeedUpdate(m_module.m_window, &rect);
					m_module.TriggerSelectionChanged();
				}
				return; // Salimos temprano (KISS)
			}
    
			if (focusChanged)
			{
				m_module.m_focusedLogicalIndex = m_module.m_items.GetLogicalIndex(currentVisual);
       
				// Crucial: Si EnsureVisible cambió la posición del scroll, todo cambió de lugar.
				// Debes hacer que m_module.EnsureVisible devuelva un bool.
				bool scrollScrolled = m_module.EnsureVisible(currentVisual);

				if (!args.ButtonState.Ctrl)
				{
					selectionChanged = m_module.SelectItemConResolver(m_module.m_focusedLogicalIndex.value(), false, args.ButtonState.Shift);
				}

				if (scrollScrolled)
				{
					// Si hubo scroll, invalidamos TODO el control (las posiciones relativas cambiaron)
					GUI::MarkAsNeedUpdate(m_module.m_window, nullptr);
				}
				else
				{
					// Optimizadísimo (Nivel AAA): No hubo scroll, solo invalidamos quirúrgicamente 
					// el ítem que perdió el foco/selección y el ítem que lo ganó.
					if (prevVisual.has_value())
					{
						Rectangle prevRect = m_module.GetItemLocalRect(*prevVisual);
						GUI::MarkAsNeedUpdate(m_module.m_window, &prevRect);
					}
          
					Rectangle currRect = m_module.GetItemLocalRect(currentVisual);
					GUI::MarkAsNeedUpdate(m_module.m_window, &currRect);
				}

				if (selectionChanged)
				{
					m_module.TriggerSelectionChanged();
				}
			}
		}

		void HeaderController::Init(Window* owner)
		{
			m_owner = owner;
		}

		void HeaderController::Append(const std::string& name, uint32_t width)
		{
			m_headers.emplace_back(name, std::max<uint32_t>(width, LISTBOX_MIN_HEADER_WIDTH));
			m_visualOrder.emplace_back(static_cast<int>(m_headers.size() - 1));
		}

		void HeaderController::Clear()
		{
			m_headers.clear();
			m_visualOrder.clear();
			m_resizeInteraction = {};
		}

		void HeaderController::SetSortState(size_t logicalColumnIndex, bool ascending)
		{
			m_sortLogicalIndex = logicalColumnIndex;
			m_isSortAscending = ascending;
		}

		uint32_t HeaderController::GetTotalWidth() const
		{
			uint32_t total = 0;
			for (const auto& header : m_headers)
			{
				auto headerWidth = m_owner->ToScale(header.Width);
				total += headerWidth;
			}
			return total;
		}

		uint32_t HeaderController::GetColumnWidth(size_t index) const
		{
			if (index >= m_headers.size())
				return 0;
			
			return m_owner->ToScale(m_headers[index].Width);
		}

		void HeaderController::Draw(Graphics& graphics, const Rectangle& visibleRect, const Point& clientPos)
		{
			auto appearance = reinterpret_cast<Appearance*>(m_owner->Appearance.get());
			auto headerHeight = m_owner->ToScale(appearance->HeadersHeight);
			auto headerHeightInt = static_cast<int>(headerHeight);
			auto startOffPos = static_cast<int>(m_owner->ToScale(m_startOffPos));
			auto leftPadding = m_owner->ToScale(m_textPadding.Left);
			auto topPadding = m_owner->ToScale(m_textPadding.Top);
			int clientWidth = static_cast<int>(visibleRect.Width);
			int sortedHeaderMargin = m_owner->ToScale(4);
			int arrowSortedHeaderSize = m_owner->ToScale(6);
			int currentX = clientPos.X - visibleRect.X + startOffPos;

			Rectangle fullHeaderRect = { clientPos.X, clientPos.Y, visibleRect.Width, headerHeight };
			ScopedClip clipping(graphics, fullHeaderRect);
			
			graphics.DrawGradientFill(fullHeaderRect, appearance->ButtonHighlightBackground, appearance->ButtonBackground);
			graphics.DrawLine(
				{ currentX - 1, clientPos.Y }, 
				{ currentX - 1, clientPos.Y + headerHeightInt - 1 },
				appearance->BoxBorderColor);
			
			for (size_t visualIdx = 0; visualIdx < m_visualOrder.size(); ++visualIdx)
			{
				auto logicalIdx = m_visualOrder[visualIdx];
				const auto& header = m_headers[logicalIdx];
				auto headerWidth = m_owner->ToScale(header.Width);
				int headerWidthInt = static_cast<int>(headerWidth);
				
				bool isHovered = visualIdx == m_hoveredVisualIndex && (!m_dragDropInteraction.m_isDraggingConfirmed || m_dragDropInteraction.m_draggedVisualIndex != visualIdx);
				bool isSortedHeader = logicalIdx == m_sortLogicalIndex;

				if (currentX + headerWidthInt >= 0 && currentX < clientWidth)
				{
					Rectangle headerRect = { currentX, 0, headerWidth, headerHeight };
					
					if (isHovered)
					{
						graphics.FillRectangle(headerRect, appearance->HighlightColor);
					}
					Rectangle textRect = headerRect;
					textRect.X += leftPadding;
					textRect.Width -= leftPadding * 2;
					if (isSortedHeader)
					{
						textRect.Width -= sortedHeaderMargin + arrowSortedHeaderSize;
					}
					DrawStringInBox(graphics, header.Text, textRect, appearance->Foreground);
					
					// vertical line
					Point p1 = { currentX + headerWidthInt - 1, 4 };
					Point p2 = { currentX + headerWidthInt - 1, headerHeightInt - 4 };
					graphics.DrawLine(p1, p2, appearance->BoxBorderColor);
					
					if (isSortedHeader)
					{
						int arrowWidth = m_owner->ToScale(4);
						int arrowLength = m_owner->ToScale(2);
						Rectangle arrowRect = headerRect;
						arrowRect.X += headerWidthInt - arrowSortedHeaderSize - sortedHeaderMargin;
						arrowRect.Width = arrowSortedHeaderSize;
						graphics.DrawArrow(arrowRect, arrowLength, arrowWidth, 
							m_isSortAscending ? Graphics::ArrowDirection::Upwards : Graphics::ArrowDirection::Downwards,
							appearance->Foreground2nd);
					}
				}
				currentX += headerWidthInt;
			}
			
			if (m_dragDropInteraction.m_draggedVisualIndex.has_value() && m_dragDropInteraction.m_isDraggingConfirmed)
			{
				auto draggedLogicalIdx = m_visualOrder[m_dragDropInteraction.m_draggedVisualIndex.value()];
				const auto& draggedHeader = m_headers[draggedLogicalIdx];
				Rectangle columnRect{ 0,0,m_owner->ToScale(draggedHeader.Width), headerHeight };
				int lineWidth = m_owner->ToScale(2);
				auto targetHeaderPosition = -visibleRect.X;
				
				auto visualColumnIdxMouse = GetVisualIndexAt(m_dragDropInteraction.m_currentMouseX, visibleRect.X);
				if (!visualColumnIdxMouse.has_value())
				{
					auto totalWidth = static_cast<int>(GetTotalWidth());
					if (m_dragDropInteraction.m_currentMouseX > startOffPos + totalWidth - visibleRect.X)
					{
						targetHeaderPosition += totalWidth;
					}
				}
				else
				{
					auto columnPosition = static_cast<int>(m_owner->ToScale(GetPositionToColumn(visualColumnIdxMouse.value())));
					auto targetVisualColumnIdWidth = static_cast<int>(m_owner->ToScale(m_headers[m_visualOrder[visualColumnIdxMouse.value()]].Width));
					targetHeaderPosition += columnPosition;
					if (m_dragDropInteraction.m_currentMouseX >= columnPosition + targetVisualColumnIdWidth / 2)
					{
						targetHeaderPosition += targetVisualColumnIdWidth;
					}
				}
				targetHeaderPosition += startOffPos;
				// dragging vertical line
				graphics.DrawLine({ targetHeaderPosition, clientPos.Y }, { targetHeaderPosition, headerHeightInt }, static_cast<float>(lineWidth), appearance->SelectionHighlightColor);

				if (!m_dragDropInteraction.m_draggingBox.IsValid())
				{
					m_dragDropInteraction.m_draggingBox.Build({ columnRect.Width, columnRect.Height }, m_owner->RootPaintHandle);
					m_dragDropInteraction.m_draggingBox.BuildFont(m_owner->DPI);
					
					m_dragDropInteraction.m_draggingBox.Begin();
					m_dragDropInteraction.m_draggingBox.DrawGradientFill({ 0,0, columnRect.Width, columnRect.Height }, appearance->Foreground, appearance->Foreground2nd);
					
					Rectangle textRect = columnRect;
					textRect.X += leftPadding;
					textRect.Width -= leftPadding * 2;
					DrawStringInBox(m_dragDropInteraction.m_draggingBox, draggedHeader.Text, textRect, appearance->Foreground);
					
					m_dragDropInteraction.m_draggingBox.Flush();
				}
				auto positionToColumn = static_cast<int>(m_owner->ToScale( GetPositionToColumn(m_dragDropInteraction.m_draggedVisualIndex.value())));
				auto newPosition = m_dragDropInteraction.m_currentMouseX - (m_dragDropInteraction.m_dragStartX - positionToColumn + visibleRect.X);
				
				Rectangle blendRect{ newPosition, clientPos.Y, columnRect.Width, columnRect.Height };
				graphics.Blend(blendRect, m_dragDropInteraction.m_draggingBox, { 0,0 }, 0.5);
			}

			// Línea horizontal inferior que separa las cabeceras de la lista
			graphics.DrawLine(
				{clientPos.X, clientPos.Y + headerHeightInt - 1},
				{clientPos.X + clientWidth, clientPos.Y + headerHeightInt - 1},
				appearance->BoxBorderColor);
		}

		bool HeaderController::OnDblClick(const ArgMouse& args, int scrollX)
		{
			auto dividerVisualIdx = GetDividerVisualIndexAt(args.Position.X, scrollX);
			if (dividerVisualIdx.has_value() && m_onRequestAutoWidth)
			{
				auto logicalIdx = m_visualOrder[dividerVisualIdx.value()];
        
				auto idealWidth = m_onRequestAutoWidth(logicalIdx);
				m_headers[logicalIdx].Width = std::max<uint32_t>(LISTBOX_MIN_HEADER_WIDTH, idealWidth + 20); // Padding
				
				GUI::MarkAsNeedUpdate(m_owner);
				return true;
			}
			return false;
		}
		
		bool HeaderController::OnMouseDown(const ArgMouse& args, int scrollX)
		{
			auto appearance = reinterpret_cast<Appearance*>(m_owner->Appearance.get());
			auto headerHeight = m_owner->ToScale(appearance->HeadersHeight);
			m_mouseDownOnHeadersRow = false;
			if (args.Position.Y >= static_cast<int>(headerHeight))
			{
				return false;
			}
			auto dividerVisualIdx = GetDividerVisualIndexAt(args.Position.X, scrollX);
			if (dividerVisualIdx.has_value())
			{
				m_resizeInteraction.m_isResizing = true;
				m_resizeInteraction.m_visualColumnIndex = dividerVisualIdx;
				m_resizeInteraction.m_startX = args.Position.X;
				auto logicalIdx = m_visualOrder[dividerVisualIdx.value()];
				m_resizeInteraction.m_startWidth = m_headers[logicalIdx].Width;
            
				GUI::Capture(m_owner);
				return true;
			}
			
			auto colVisualIdx = GetVisualIndexAt(args.Position.X, scrollX);
			if (colVisualIdx.has_value())
			{
				m_dragDropInteraction.m_draggedVisualIndex = colVisualIdx;
				m_dragDropInteraction.m_dragStartX = args.Position.X;
				m_dragDropInteraction.m_currentMouseX = args.Position.X;
				m_dragDropInteraction.m_isDraggingConfirmed = false;
				
				GUI::Capture(m_owner);
				return true;
			}
			m_mouseDownOnHeadersRow = true;
			return true;
		}

		bool HeaderController::OnMouseMove(const ArgMouse& args, int scrollX)
		{
			auto appearance = reinterpret_cast<Appearance*>(m_owner->Appearance.get());
			auto headerHeight = static_cast<int>(m_owner->ToScale(appearance->HeadersHeight));
			
			Rectangle fullHeaderRect = {0,0, m_owner->ClientSize.Width, static_cast<uint32_t>(headerHeight)};
			if (m_resizeInteraction.m_isResizing)
			{
				int deltaX = args.Position.X - m_resizeInteraction.m_startX;
				int newWidthLocal = m_owner->ToScale(static_cast<int>(m_resizeInteraction.m_startWidth)) + deltaX;
				auto newWidth = m_owner->ToDownwardScale(std::max<int>(newWidthLocal,0));
				
				auto logicalIdx = m_visualOrder[m_resizeInteraction.m_visualColumnIndex.value()];
				m_headers[logicalIdx].Width = std::max<uint32_t>(LISTBOX_MIN_HEADER_WIDTH, newWidth);
				
				GUI::MarkAsNeedUpdate(m_owner);
				return true;
			}
			
			if (m_dragDropInteraction.m_draggedVisualIndex.has_value())
			{
				m_dragDropInteraction.m_currentMouseX = args.Position.X;
				if (!m_dragDropInteraction.m_isDraggingConfirmed && std::abs(m_dragDropInteraction.m_currentMouseX - m_dragDropInteraction.m_dragStartX) > m_owner->ToScale(5))
				{
					m_dragDropInteraction.m_isDraggingConfirmed = true;
				}

				if (m_dragDropInteraction.m_isDraggingConfirmed)
				{
					GUI::MarkAsNeedUpdate(m_owner, &fullHeaderRect);
					return true;
				}
			}
			
			if (args.Position.Y < 0 || args.Position.Y >= headerHeight)
			{
				if (m_hoveredVisualIndex.has_value())
				{
					m_hoveredVisualIndex.reset();
					GUI::MarkAsNeedUpdate(m_owner, &fullHeaderRect);
				}
				return m_mouseDownOnHeadersRow;
			}

			auto hoveredVisual = GetVisualIndexAt(args.Position.X, scrollX);
			auto hoveredDivVisual = GetDividerVisualIndexAt(args.Position.X, scrollX);

			if (hoveredDivVisual.has_value())
			{
				GUI::ChangeCursor(m_owner, Cursor::SizeWE);
				m_resizeInteraction.m_isHoveringDivider = true;
			}
			else
			{
				GUI::ChangeCursor(m_owner, Cursor::Default);
				m_resizeInteraction.m_isHoveringDivider = false;
			}
			
			if (!m_dragDropInteraction.m_isDraggingConfirmed && !m_resizeInteraction.m_isResizing)
			{
				if (m_hoveredVisualIndex != hoveredVisual)
				{
					m_hoveredVisualIndex = hoveredVisual;
					GUI::MarkAsNeedUpdate(m_owner, &fullHeaderRect);
				}
			}
			
			return hoveredVisual.has_value() || hoveredDivVisual.has_value();
		}

		bool HeaderController::OnMouseUp(const ArgMouse& args, int scrollX)
		{
			bool handled = false;

			if (m_resizeInteraction.m_isResizing)
			{
				m_resizeInteraction.m_isResizing = false;
				handled = true;
			}
			else if (m_dragDropInteraction.m_draggedVisualIndex.has_value())
			{
				if (m_dragDropInteraction.m_isDraggingConfirmed)
				{
					auto dropIndex = GetVisualIndexAt(args.Position.X, scrollX);
					if (dropIndex.has_value() && dropIndex != m_dragDropInteraction.m_draggedVisualIndex)
					{
						std::swap(m_visualOrder[m_dragDropInteraction.m_draggedVisualIndex.value()], m_visualOrder[dropIndex.value()]);
						if (m_onHeadersReordered)
						{
							m_onHeadersReordered();
						}
					}
					
					m_dragDropInteraction.m_draggingBox.Release();
				}
				else
				{
					auto logicalIdx = m_visualOrder[m_dragDropInteraction.m_draggedVisualIndex.value()];
					if (m_onHeaderClicked)
					{
						m_onHeaderClicked(logicalIdx);
					}
				}

				m_dragDropInteraction.m_draggedVisualIndex.reset();
				m_dragDropInteraction.m_isDraggingConfirmed = false;
				handled = true;
			}

			if (handled)
			{
				GUI::ReleaseCapture(m_owner);
				GUI::MarkAsNeedUpdate(m_owner);
			}
			if (m_mouseDownOnHeadersRow)
			{
				m_mouseDownOnHeadersRow = false;
				return true;
			}
			return handled;
		}

		bool HeaderController::OnMouseLeave()
		{
			if (m_resizeInteraction.m_isHoveringDivider && !m_resizeInteraction.m_isResizing)
			{
				GUI::ChangeCursor(m_owner, Cursor::Default);
				m_resizeInteraction.m_isHoveringDivider = false;
			}
			if (m_hoveredVisualIndex.has_value())
			{
				m_hoveredVisualIndex.reset();
				GUI::MarkAsNeedUpdate(m_owner);
			}
			return false;
		}
		
		void HeaderController::SetTextPadding(int top, int bottom, int left, int right)
		{
			m_textPadding = { top, bottom, left, right };
		}

		uint32_t HeaderController::GetPositionToColumn(size_t visualIdx) const
		{
			uint32_t position = 0;
			for (size_t i = 0; i < m_visualOrder.size(); ++i)
			{
				auto logicalIdx = m_visualOrder[i];
				if (i  == visualIdx)
				{
					return position;
				}
				position += m_headers[logicalIdx].Width;
			}
			return position;
		}

		void HeaderController::DrawStringInBox(Graphics& graphics, const std::string& str, const Rectangle& boxBounds,
		                                       const Color& textColor)
		{
			auto textExtent = graphics.GetTextExtent(str);
			if (boxBounds.X + static_cast<int>(textExtent.Width) < 0)
			{
				return;
			}

			if (textExtent.Width < boxBounds.Width)
			{
				graphics.DrawString({ boxBounds.X, boxBounds.Y + (static_cast<int>(boxBounds.Height - textExtent.Height) >> 1) }, str, textColor);
				return;
			}

			auto ellipsisTextExtent = graphics.GetTextExtent("...").Width;
			for (size_t i = str.size(); i >= 1; --i)
			{
				auto subStr = str.substr(0, i);// +"...";
				auto subTextExtent = graphics.GetTextExtent(subStr).Width;
				if (static_cast<int>(subTextExtent + ellipsisTextExtent) <= static_cast<int>(boxBounds.Width) - 2)
				{
					graphics.DrawString({ boxBounds.X, boxBounds.Y + (static_cast<int>(boxBounds.Height - textExtent.Height) >> 1) }, subStr + "...", textColor);
					break;
				}
			}
		}

		std::optional<size_t> HeaderController::GetVisualIndexAt(int mouseX, int scrollX) const
		{
			int currentX = -scrollX + static_cast<int>(m_owner->ToScale(m_startOffPos));
			for (size_t visualIdx = 0; visualIdx < m_visualOrder.size(); ++visualIdx)
			{
				auto logicalIdx = m_visualOrder[visualIdx];
				auto headerWidth = static_cast<int>(m_owner->ToScale(m_headers[logicalIdx].Width));

				if (mouseX >= currentX && mouseX < currentX + headerWidth)
				{
					return visualIdx;	
				}
                
				currentX += headerWidth;
			}
			return std::nullopt;
		}

		std::optional<size_t> HeaderController::GetDividerVisualIndexAt(int mouseX, int scrollX) const
		{
			int currentX = -scrollX + static_cast<int>(m_owner->ToScale(m_startOffPos));
			int tolerance = m_owner->ToScale(4);
    
			for (size_t visualIdx = 0; visualIdx < m_visualOrder.size(); ++visualIdx)
			{
				auto logicalIdx = m_visualOrder[visualIdx];
				auto headerWidth = static_cast<int>(m_owner->ToScale(m_headers[logicalIdx].Width));
				currentX += headerWidth;
        
				if (std::abs(mouseX - currentX) <= tolerance)
				{
					return visualIdx;	
				}
			}
			return std::nullopt;
		}

		void Reactor::Module::Erase(ListBoxItem item)
		{
			std::vector<size_t> currentSelected = m_selectionController.GetSelectedItems();
			m_selectionController.Clear();
			
			for (size_t selIdx : currentSelected)
			{
				if (selIdx == item.m_logicalIndex) 
				{
					continue;
				}
				if (selIdx > item.m_logicalIndex) 
				{
					m_selectionController.SetSelected(selIdx - 1, true); 
				}
				else 
				{
					m_selectionController.SetSelected(selIdx, true);
				}
			}
			m_items.Erase(item.m_logicalIndex);
			if (m_focusedLogicalIndex == item.m_logicalIndex)
			{
				m_focusedLogicalIndex.reset();
			}
			
			UpdateScrollData();
			GUI::MarkAsNeedUpdate(m_window);
		}

		void Reactor::Module::Erase(std::vector<ListBoxItem>& items)
		{
			if (items.empty())
			{
				return;
			}

			std::vector<size_t> indicesToErase;
			indicesToErase.reserve(items.size());
	
			for (auto& item : items)
			{
				indicesToErase.emplace_back(item.m_logicalIndex); 
			}
			
			std::sort(indicesToErase.begin(), indicesToErase.end(), std::greater<size_t>());
			
			std::vector<size_t> currentSelected = m_selectionController.GetSelectedItems();
			m_selectionController.Clear();

			for (size_t logicalIndex : indicesToErase)
			{
				for (size_t& selIdx : currentSelected)
				{
					if (selIdx == logicalIndex) 
					{
						selIdx = static_cast<size_t>(-1);
					}
					else if (selIdx != static_cast<size_t>(-1) && selIdx > logicalIndex) 
					{
						selIdx--;
					}
				}
				
				m_items.Erase(logicalIndex);
				if (m_focusedLogicalIndex == logicalIndex)
				{
					m_focusedLogicalIndex.reset();
				}
			}

			for (size_t selIdx : currentSelected)
			{
				if (selIdx != static_cast<size_t>(-1))
				{
					m_selectionController.SetSelected(selIdx, true);
				}
			}

			UpdateScrollData();
			
			GUI::MarkAsNeedUpdate(m_window);
		}

		void Reactor::Module::EnableMultiselection(bool enabled)
		{
			if (m_selectionController.IsMultiSelect() == enabled)
			{
				return;
			}
			
			m_selectionController.SetMultiSelect(enabled);
		}

		void Reactor::Module::ResetScrollOffset()
		{
			m_scrollableView->SetScrollToX(0);
			m_scrollableView->SetScrollToY(0);
		}

		void Reactor::Module::AppendHeader(const std::string& text, uint32_t width)
		{
			m_headers.Append(text, width);
			
			UpdateScrollData();
			
			GUI::MarkAsNeedUpdate(m_window);
		}

		ListBoxItem Reactor::Module::Append(const std::string& text)
		{
			auto logicalIndex = m_items.Append(text);
			UpdateScrollData();

			GUI::MarkAsNeedUpdate(m_window);
			
			return{logicalIndex, &m_items};
		}

		ListBoxItem Reactor::Module::Append(std::initializer_list<std::string> texts)
		{
			auto logicalIndex = m_items.Append(texts);
			
			UpdateScrollData();
			
			GUI::MarkAsNeedUpdate(m_window);
			
			return{logicalIndex, &m_items};
		}

		ListBoxItem Reactor::Module::At(size_t index)
		{
			if (index >= m_items.GetCount())
			{
				return {0, nullptr};
			}
			return ListBoxItem{ index, &m_items };
		}

		void Reactor::Module::Clear()
		{
			m_items.Clear();
			m_selectionController.Clear();
			m_hoveredIndex.reset();
			m_focusedLogicalIndex.reset();
			
			ResetScrollOffset();
			UpdateScrollData();
			
			// El control ahora está vacío, invalidamos toda su área local
			GUI::MarkAsNeedUpdate(m_window);
		}

		void Reactor::Module::ClearHeaders()
		{
			m_headers.Clear();
			
			UpdateScrollData();
			GUI::MarkAsNeedUpdate(m_window);
		}

		void Reactor::Module::UpdateScrollData()
		{
			if (!m_scrollableView)
			{
				return;
			}
			
			auto appearance = reinterpret_cast<Appearance*>(m_window->Appearance.get());
			auto headerHeight = m_window->ToScale(appearance->HeadersHeight);
			auto clientArea = m_control->GetClientArea();
			
			m_scrollableView->SetViewRect(clientArea);
			m_scrollableView->SetViewPadding(static_cast<int>(headerHeight), 0, 0, 0);
			
			auto four = m_window->ToScale(4u);
			
			Size contentSize;
			contentSize.Width = four;
			for (const auto& header : m_headers.GetHeaders())
			{
				auto headerWidth = m_window->ToScale(header.Width);
				contentSize.Width += headerWidth;
			}
			contentSize.Width += four;
			
			auto innerMargin = m_window->ToScale(2u);
			int itemHeight = static_cast<int>(m_window->ToScale(appearance->ListItemHeight));
			int itemHeightWithMargin = static_cast<int>(itemHeight + innerMargin * 2u);
			contentSize.Height = static_cast<uint32_t>(m_items.GetCount()) * itemHeightWithMargin + headerHeight;

			m_scrollableView->SetContentSize(contentSize);
		}
		
		void Reactor::Module::SortHeader(size_t columnIndex, bool ascending)
		{
			if (m_items.IsEmpty() || columnIndex >= m_headers.GetColumnCount())
			{
				return;
			}
			m_items.Sort(columnIndex, ascending);
		}

		void Reactor::Module::InitScrollableView()
		{
			auto appearance = reinterpret_cast<Appearance*>(m_window->Appearance.get());
			m_scrollableView = std::make_unique<ScrollableView>(m_window);
        
			m_scrollableView->SetScrollStep(static_cast<int>(m_window->ToScale(appearance->ListItemHeight)), 20);
			m_scrollableView->SetOnScrollChange([this]()
			{
				GUI::MarkAsNeedUpdate(m_window);
			});
		}

		bool Reactor::Module::EnsureVisible(size_t visualIndex)
		{
			auto appearance = reinterpret_cast<Appearance*>(m_window->Appearance.get());
			auto innerMargin = m_window->ToScale(2u);
			int itemHeight = static_cast<int>(m_window->ToScale(appearance->ListItemHeight));
			int itemHeightWithMargin = static_cast<int>(itemHeight + innerMargin * 2u);
			int itemY = static_cast<int>(visualIndex) * itemHeightWithMargin;
			int currentScrollY = m_scrollableView->GetScrollOffset().Y;
            
			int visibleHeight = static_cast<int>(m_scrollableView->GetClientArea().Height) - static_cast<int>(m_window->ToScale(appearance->HeadersHeight));

			if (itemY < currentScrollY)
			{
				m_scrollableView->SetScrollToY(itemY);
				return true;
			}
			if (itemY + itemHeightWithMargin > currentScrollY + visibleHeight)
			{
				m_scrollableView->SetScrollToY(itemY + itemHeightWithMargin - visibleHeight);
				return true;
			}
			return false;
		}

		bool Reactor::Module::SelectItemConResolver(size_t logicalIndex, bool isCtrl, bool isShift)
		{
			auto rangeResolver = [&](size_t anchorLogical, size_t currentLogical)
			{
				std::vector<size_t> result;
				size_t anchorVisual = m_items.GetVisualIndex(anchorLogical);
				size_t currentVisual = m_items.GetVisualIndex(currentLogical);
                
				size_t start = std::min<size_t>(anchorVisual, currentVisual);
				size_t end = std::max<size_t>(anchorVisual, currentVisual);
                
				for (size_t i = start; i <= end; ++i)
				{
					result.emplace_back(m_items.GetLogicalIndex(i));
				}
				return result;
			};

			return m_selectionController.Select(logicalIndex, isCtrl, isShift, rangeResolver);
		}

		void Reactor::Module::ProcessLassoIntersection(bool ctrlPressed)
		{
			auto appearance = reinterpret_cast<Appearance*>(m_window->Appearance.get());
			
			Rectangle lassoRect = m_lassoSelection.GetRect();
    
			auto scrollOffset = m_scrollableView->GetVisibleRect();
			int scrollY = scrollOffset.Y;
			int headerHeight = static_cast<int>(m_window->ToScale(appearance->HeadersHeight));
	    
			m_selectionController.RestoreSnapshot();

			if (lassoRect.Y + static_cast<int>(lassoRect.Height) <= headerHeight)
			{
				return;
			}
			
			int y1 = lassoRect.Y - headerHeight + scrollY;
			int y2 = lassoRect.Y + static_cast<int>(lassoRect.Height) - headerHeight + scrollY;
			int topAbsoluteY = std::min<int>(y1, y2);
			int bottomAbsoluteY = std::max<int>(y1, y2);
			
			topAbsoluteY = std::max<int>(topAbsoluteY, 0);
			
			auto innerMargin = m_window->ToScale(2u);
			int itemHeight = static_cast<int>(m_window->ToScale(appearance->ListItemHeight));
			int itemHeightWithMargin = static_cast<int>(itemHeight + innerMargin * 2u);

			int startVisualIndex = topAbsoluteY / itemHeightWithMargin;
			int endVisualIndex = bottomAbsoluteY / itemHeightWithMargin;

			int maxIndex = static_cast<int>(m_items.GetCount()) - 1;
			if (maxIndex < 0)
			{
				return;
			}

			for (int visualIdx = startVisualIndex; visualIdx <= endVisualIndex; ++visualIdx)
			{
				if (visualIdx >= 0 && visualIdx <= maxIndex)
				{
					size_t logicalIdx = m_items.GetLogicalIndex(visualIdx);
					if (logicalIdx != static_cast<size_t>(-1))
					{
						if (ctrlPressed)
						{
							bool wasSelected = m_selectionController.IsSelected(logicalIdx);
							m_selectionController.SetSelected(logicalIdx, !wasSelected);
						}
						else
						{
							m_selectionController.SetSelected(logicalIdx, true);
						}
					}
				}
			}
		}
		
		
		void Reactor::Module::DrawStringInBox(Graphics& graphics, const std::string& str, const Rectangle& boxBounds, const Color& textColor)
		{
			auto textExtent = graphics.GetTextExtent(str);
			if (boxBounds.X + static_cast<int>(textExtent.Width) < 0)
			{
				return;
			}

			if (textExtent.Width < boxBounds.Width)
			{
				graphics.DrawString({ boxBounds.X, boxBounds.Y + (static_cast<int>(boxBounds.Height - textExtent.Height) >> 1) }, str, textColor);
				return;
			}

			auto ellipsisTextExtent = graphics.GetTextExtent("...").Width;
			for (size_t i = str.size(); i >= 1; --i)
			{
				auto subStr = str.substr(0, i);// +"...";
				auto subTextExtent = graphics.GetTextExtent(subStr).Width;
				if (static_cast<int>(subTextExtent + ellipsisTextExtent) <= static_cast<int>(boxBounds.Width) - 2)
				{
					graphics.DrawString({ boxBounds.X, boxBounds.Y + (static_cast<int>(boxBounds.Height - textExtent.Height) >> 1) }, subStr + "...", textColor);
					break;
				}
			}
		}

		void Reactor::Module::DrawList(Graphics& graphics)
		{
			if (m_items.IsEmpty())
			{
				return;
			}
			auto appearance = reinterpret_cast<Appearance*>(m_window->Appearance.get());
			int headerHeight = static_cast<int>(m_window->ToScale(appearance->HeadersHeight));
			
			Rectangle viewportRect = m_scrollableView->GetClientArea();
			viewportRect.Y += headerHeight;
			viewportRect.Height -= headerHeight;
			graphics.SetClipping(viewportRect);
			
			Rectangle visibleRect = m_scrollableView->GetVisibleRect();
			
			auto innerMargin = m_window->ToScale(2u);
			int itemHeight = static_cast<int>(m_window->ToScale(appearance->ListItemHeight));
			int itemHeightWithMargin = static_cast<int>(itemHeight + innerMargin * 2u);
			int listVisibleY = std::max<int>(0, visibleRect.Y - headerHeight);
			int startIndex = listVisibleY / itemHeightWithMargin;
			int endIndex = std::min<int>(static_cast<int>(m_items.GetCount()), (listVisibleY + static_cast<int>(visibleRect.Height)) / itemHeight + 1);

			int currentY = (startIndex * itemHeightWithMargin) + viewportRect.Y - visibleRect.Y;
    
			for (int i = startIndex; i < endIndex; ++i)
			{
				Rectangle rowRect = m_scrollableView->GetClientArea();
				rowRect.Y = currentY + static_cast<int>(innerMargin);
				rowRect.Height = itemHeight;

				rowRect.X -= visibleRect.X; 
        
				DrawRowBackground(graphics, i, rowRect);
				DrawRowContent(graphics, i, rowRect);

				currentY += itemHeightWithMargin;
			}
			
			graphics.EndClipping();
		}
		
		void Reactor::Module::DrawRowBackground(Graphics& graphics, int visualIndex, const Rectangle& rowRect)
		{
			auto appearance = reinterpret_cast<Appearance*>(m_window->Appearance.get());
			Color bgColor = appearance->BoxBackground;
			size_t logicalIndex = m_items.GetLogicalIndex(visualIndex);
			bool isSelected = m_selectionController.IsSelected(logicalIndex); 
			bool isHovered = (visualIndex == m_hoveredIndex);
			bool isFocused = (logicalIndex == m_focusedLogicalIndex);
			
			if (isSelected)
			{
				bgColor = appearance->SelectionHighlightColor;
			}
			else if (isHovered)
			{
				bgColor = appearance->HighlightColor;
			}
			
			if (bgColor != appearance->BoxBackground)
			{
				Rectangle bgRect = rowRect;
				bgRect.Width = std::max<uint32_t>(rowRect.Width, m_headers.GetTotalWidth()); 
				graphics.FillRectangle(bgRect, bgColor);
			}
			if (isFocused)
			{
				Rectangle bgRect = rowRect;
				bgRect.Width = std::max<uint32_t>(rowRect.Width, m_headers.GetTotalWidth()); 
				graphics.DrawRectangle(bgRect, appearance->Foreground);
			}
		}

		void Reactor::Module::DrawRowContent(Graphics& graphics, int visualRowIndex, const Rectangle& rowRect)
		{
			size_t logicalRowIndex = m_items.GetLogicalIndex(visualRowIndex);
			const auto* item = m_items.GetItemSafely(logicalRowIndex);
			const auto& headers = m_headers.GetHeaders();
			size_t columnCount = m_headers.GetColumnCount();
			
			const Image* icon = item->m_icon ? &item->m_icon : nullptr;
			
			int currentX = rowRect.X;
			auto cellHeight = rowRect.Height;
			
			auto isSelected = m_selectionController.IsSelected(logicalRowIndex);
			
			for (size_t visualCol = 0; visualCol < columnCount; ++visualCol)
			{
				auto logicalCol = m_headers.GetLogicalIndex(visualCol);
				auto colWidth = m_window->ToScale(headers[logicalCol].Width);
				Rectangle cellRect = { currentX, rowRect.Y, colWidth, cellHeight };
				
				if (logicalCol < item->m_cells.size())
				{
					const std::string& cellText = item->m_cells[logicalCol].m_text;
					DrawCell(graphics, cellRect, cellText, isSelected, logicalCol == 0 ? icon : nullptr, logicalCol == 0 && m_items.ShouldDrawImages());
				}
				currentX += static_cast<int>(colWidth);
			}
		}

		void Reactor::Module::DrawCell(Graphics& graphics, const Rectangle& rect, const std::string& text, bool isRowSelected, const Image* icon, bool drawIconsForColumn)
		{
			auto appearance = reinterpret_cast<Appearance*>(m_window->Appearance.get());
			// Padding
			auto textPadding= m_window->ToScale(3u);
			Rectangle textRect = rect;
			textRect.X += static_cast<int>(textPadding); 
			textRect.Width -= textPadding * 2u;
			
			auto listItemIconSize = m_window->ToScale(appearance->ListItemIconSize);
			auto listItemIconMargin = m_window->ToScale(appearance->ListItemIconMargin);
			if (icon)
			{
				auto iconSize = icon->GetSize();
				Rectangle destRect = rect;
				destRect.X += static_cast<int>(listItemIconMargin);
				destRect.Y += static_cast<int>(rect.Height - listItemIconSize) / 2;
				destRect.Height = listItemIconSize;
				destRect.Width = listItemIconSize;
				
				icon->Paste(iconSize.ToRectangle(), graphics, destRect);
			}
			
			if (drawIconsForColumn)
			{
				textRect.X += static_cast<int>(listItemIconMargin + listItemIconSize);
				textRect.Width -= listItemIconMargin * 2u + listItemIconSize;
			}

			Color textColor = isRowSelected ? appearance->HighlightTextColor : appearance->Foreground;
			DrawStringInBox(graphics, text, textRect, textColor);
		}

		Rectangle Reactor::Module::GetItemLocalRect(size_t visualIndex)
		{
			auto appearance = reinterpret_cast<Appearance*>(m_window->Appearance.get());
			int headerHeight = static_cast<int>(m_window->ToScale(appearance->HeadersHeight));
			auto innerMargin = m_window->ToScale(2);
			int itemHeight = static_cast<int>(m_window->ToScale(appearance->ListItemHeight));
			int itemHeightWithMargin = static_cast<int>(itemHeight + innerMargin * 2u);
			auto scrollOffset = m_scrollableView->GetVisibleRect();
			Rectangle viewportRect = m_scrollableView->GetClientArea();
			
			Rectangle rect;
			rect.X = viewportRect.X;
			rect.Y = headerHeight + (static_cast<int>(visualIndex) * itemHeightWithMargin) + innerMargin - scrollOffset.Y + viewportRect.Y;
			rect.Width = m_window->ClientSize.Width;
			rect.Height = itemHeight;

			return rect;
		}

		Rectangle Reactor::Module::GetCellLocalRect(size_t visualIndex, size_t columnIndex)
		{
			Rectangle rowRect = GetItemLocalRect(visualIndex);
    
			// Sumamos el ancho de las columnas anteriores para obtener el desplazamiento en X
			int32_t offsetX = 0;
			for (size_t i = 0; i < columnIndex; ++i)
			{
				offsetX += static_cast<int>(m_headers.GetColumnWidth(i));
			}
    
			rowRect.X += offsetX;
			rowRect.Width = m_headers.GetColumnWidth(columnIndex);
    
			return rowRect;
		}

		void Reactor::Module::TriggerSelectionChanged()
		{
			ArgListBox arguments { GetSelectedItems() };
			auto events = reinterpret_cast<Events*>(m_window->Events.get());
			events->SelectionChanged.Emit(arguments);
		}

		std::vector<ListBoxItem> Reactor::Module::GetSelectedItems()
		{
			std::vector<ListBoxItem> result;
    
			std::vector<size_t> logicalIndices = m_selectionController.GetSelectedItems();
			if (logicalIndices.empty())
			{
				return result;
			}
			
			std::sort(logicalIndices.begin(), logicalIndices.end(), [this](size_t logicalA, size_t logicalB) 
			{
				return m_items.GetVisualIndex(logicalA) < m_items.GetVisualIndex(logicalB);
			});

			result.reserve(logicalIndices.size());
			for (size_t logicalIdx : logicalIndices)
			{
				result.emplace_back(logicalIdx, &m_items);
			}

			return result;
		}

		void ListBoxItem::SetIcon(const Image& image) const
		{
			auto item = m_collection->GetItemSafely(m_logicalIndex);
			if (!item)
			{
				return;
			}
			item->m_icon = image;
			if (image)
			{
				m_collection->EnableImages(true);
			}
			m_collection->NotifyItemModified(m_logicalIndex);
		}

		void ListBoxItem::SetText(size_t columnIndex, const std::string& text) const
		{			
			auto target = m_collection->GetItemSafely(m_logicalIndex);
			if (!target || columnIndex >= target->m_cells.size())
			{
				return;
			}
			
			if (target->m_cells[columnIndex].m_text == text)
			{
				return;
			}
			
			target->m_cells[columnIndex].m_text = text;
			m_collection->NotifyItemModified(m_logicalIndex);
		}

		std::string ListBoxItem::GetText(size_t columnIndex) const
		{
			auto target = m_collection->GetItemSafely(m_logicalIndex);
			if (!target || columnIndex >= target->m_cells.size())
			{
				return {};
			}
			
			return m_collection->GetItemSafely(m_logicalIndex)->m_cells[columnIndex].m_text;
		}

		std::any& ListBoxItem::UserData()
		{
			return m_collection->GetItemSafely(m_logicalIndex)->m_userData;
		}

		const std::any& ListBoxItem::UserData() const
		{
			return m_collection->GetItemSafely(m_logicalIndex)->m_userData;
		}
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
		GetReactor().GetModule().AppendHeader(name, width);
	}

	ListBox::ListBoxItem ListBox::Append(const std::string& text)
	{
		return GetReactor().GetModule().Append(text);
	}

	ListBox::ListBoxItem ListBox::Append(std::initializer_list<std::string> texts)
	{
		return GetReactor().GetModule().Append(texts);
	}

	ListBox::ListBoxItem ListBox::At(size_t logicalIndex)
	{
		return GetReactor().GetModule().At(logicalIndex);
	}

	void ListBox::Clear()
	{
		GetReactor().GetModule().Clear();
	}

	void ListBox::ClearHeaders()
	{
		GetReactor().GetModule().ClearHeaders();
	}

	void ListBox::Erase(ListBoxItem item)
	{
		GetReactor().GetModule().Erase(item);
	}

	void ListBox::Erase(std::vector<ListBoxItem>& items)
	{
		GetReactor().GetModule().Erase(items);
	}

	void ListBox::EnableMultiselection(bool enabled)
	{
		GetReactor().GetModule().EnableMultiselection(enabled);
	}

	std::vector<ListBox::ListBoxItem> ListBox::GetSelected()
	{
		return GetReactor().GetModule().GetSelectedItems();
	}
}
