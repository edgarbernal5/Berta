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

namespace Berta
{
	namespace ReactorCore::ListBox
	{
		size_t ItemCollection::Append(const std::string& text)
		{
			List::Item newItem;
			newItem.m_cells.push_back({ text });
        
			m_items.push_back(std::move(newItem));
			size_t newIndex = m_items.size() - 1;
        
			m_visualMap.push_back(newIndex);
			
			TriggerChanged();
			return newIndex;
		}

		size_t ItemCollection::Append(std::initializer_list<std::string> texts)
		{
			List::Item newItem;
			for (const auto& text : texts)
			{
				newItem.m_cells.push_back({ text });
			}
        
			m_items.push_back(std::move(newItem));
			size_t newIndex = m_items.size() - 1;
			m_visualMap.push_back(newIndex);
        
			TriggerChanged();
			return newIndex;
		}

		void ItemCollection::Clear()
		{
			if (!m_items.empty())
			{
				m_items.clear();
				m_visualMap.clear();
				TriggerChanged();
			}
		}

		void ItemCollection::RemoveAt(size_t index)
		{
			if (index < m_items.size())
			{
				m_items.erase(m_items.begin() + index);
				m_visualMap.erase(m_visualMap.begin() + index);
				TriggerChanged();
			}
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
			if (m_items.empty()) return;
			
			std::sort(m_visualMap.begin(), m_visualMap.end(), [&](size_t logicalA, size_t logicalB) 
			{
				const auto& itemA = m_items[logicalA];
				const auto& itemB = m_items[logicalB];

				std::string textA = (columnIndex < itemA.m_cells.size()) ? itemA.m_cells[columnIndex].m_text : "";
				std::string textB = (columnIndex < itemB.m_cells.size()) ? itemB.m_cells[columnIndex].m_text : "";

				return ascending ? (textA < textB) : (textA > textB);
			});
			TriggerChanged();
		}

		void ItemCollection::ResetSort()
		{
			for (size_t i = 0; i < m_visualMap.size(); ++i)
			{
				m_visualMap[i] = i;
			}
			TriggerChanged();
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
			// Buscamos el índice lógico dentro de nuestro mapa visual
			auto it = std::find(m_visualMap.begin(), m_visualMap.end(), logicalIndex);
    
			if (it != m_visualMap.end())
			{
				// std::distance nos da la posición (el índice visual real en pantalla)
				return static_cast<size_t>(std::distance(m_visualMap.begin(), it));
			}
    
			// Retornamos -1 (el máximo valor de size_t) si el elemento no está mapeado
			return static_cast<size_t>(-1);
		}

		void ItemCollection::NotifyItemModified()
		{
			TriggerChanged();
		}

		void ItemCollection::TriggerChanged()
		{
			if (m_onChanged)
			{
				m_onChanged();
			}
		}

		void Reactor::Init(ControlBase& control, Graphics* graphics)
		{
			m_control = &control;
			m_module.m_window = control.Handle();

			m_module.m_appearance = reinterpret_cast<ReactorCore::ListBox::Appearance*>(m_module.m_window->Appearance.get());

			m_module.m_headers.Init(m_module.m_window);
			
			m_module.InitScrollableView();
			m_module.UpdateScrollData();
		}

		void Reactor::Update(Graphics& graphics)
		{
			graphics.DrawRectangle(m_module.m_window->ClientSize.ToRectangle(), m_module.m_window->Appearance->BoxBackground, true);
			
			m_module.m_headers.Draw(graphics, m_module.m_scrollableView->GetVisibleRect(), m_module.m_scrollableView->GetScrollOffset().X);
			
			Rectangle clientArea = m_module.m_scrollableView->GetClientArea();
			int headerHeight = m_module.m_appearance->HeadersHeight;
			clientArea.Y += headerHeight;
			clientArea.Height = std::max<int>(0, static_cast<int>(clientArea.Height) - headerHeight);
			
			m_module.DrawList(graphics);
			
			if (m_module.m_lassoSelection.IsActive())
			{
				//Color fillColor = Color(m_module.m_appearance->SelectionColor, 128); // Semitransparente
				//Color borderColor = m_module.m_appearance->SelectionColor;
				//m_module.m_lassoSelection.Draw(graphics, fillColor, borderColor);
			}
			/*
			//BT_CORE_TRACE << " -- Listbox Update() " << std::endl;
			auto enabled = m_control->GetEnabled();
			auto scrollOffset = m_module.m_scrollableView->GetScrollOffset();
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

				Rectangle blendRect{ startPoint.X + scrollOffset.X, startPoint.Y + scrollOffset.Y, boxSize.Width, boxSize.Height };
				graphics.Blend(blendRect, selectionBox, { 0,0 }, 0.5);
			}

			if (m_module.m_viewport.m_needHorizontalScroll && m_module.m_viewport.m_needVerticalScroll)
			{
				auto scrollSize = m_module.m_window->ToScale(m_module.m_window->Appearance->ScrollBarSize);
				graphics.DrawRectangle({ (int)(m_module.m_window->ClientSize.Width - scrollSize) - 1, (int)(m_module.m_window->ClientSize.Height - scrollSize) - 1, scrollSize, scrollSize }, m_module.m_window->Appearance->Background, true);
			}
			graphics.DrawRectangle(m_module.m_window->ClientSize.ToRectangle(), enabled ? m_module.m_window->Appearance->BoxBorderColor : m_module.m_window->Appearance->BoxBorderDisabledColor, false);
			*/
		}

		void Reactor::DblClick(Graphics& graphics, const ArgMouse& args)
		{
			/*
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
				maxCellWidth = std::max<uint32_t>(cellWidth, maxCellWidth);
			}
			maxCellWidth = m_module.m_window->ToDownwardScale(maxCellWidth);
			auto leftMarginTextHeader = 5u;
			maxCellWidth += leftMarginTextHeader * 2u;

			auto newWidth = (std::max)(maxCellWidth, LISTBOX_MIN_HEADER_WIDTH);
			bool needUpdate = newWidth != m_module.m_headers.m_headers[selectedHeader].m_bounds.Width;
			if (!needUpdate)
				return;

			m_module.m_headers.m_headers[selectedHeader].m_bounds.Width = newWidth;
			m_module.CalculateViewport(m_module.m_viewport);
			m_module.BuildHeaderBounds(selectedHeader);

			m_module.UpdateScrollData();

			hoveredArea = m_module.DetermineHoverArea(args.Position);
			if (hoveredArea != InteractionArea::HeaderSplitter)
			{
				GUI::ChangeCursor(m_module.m_window, Cursor::Default);
			}
			m_module.m_pressedArea = InteractionArea::None;
			m_module.m_hoveredArea = hoveredArea;

			GUI::MarkAsNeedUpdate(m_module.m_window);
			*/
		}

		void Reactor::Resize(Graphics& graphics, const ArgResize& args)
		{
			m_module.CalculateViewport(m_module.m_viewport);

			m_module.BuildHeaderBounds();
			m_module.BuildListItemBounds();
			
			m_module.UpdateScrollData();
		}

		void Reactor::MouseDown(Graphics& graphics, const ArgMouse& args)
		{
			auto scrollOffset = m_module.m_scrollableView->GetVisibleRect();

			if (m_module.m_headers.OnMouseDown(args, scrollOffset.X))
			{
				return;
			}

			int headerHeight = static_cast<int>(m_module.m_appearance->HeadersHeight);
			int absoluteY = args.Position.Y - headerHeight + scrollOffset.Y;
			
			size_t clickedVisualIndex = absoluteY / m_module.m_appearance->ListItemHeight;

			if (clickedVisualIndex < m_module.m_items.GetCount())
			{
				size_t clickedLogicalIndex = m_module.m_items.GetLogicalIndex(clickedVisualIndex);
				bool isCtrl = m_module.m_ctrlPressed;
				bool isShift = m_module.m_shiftPressed;

				auto rangeResolver = [&](size_t anchorLogical, size_t currentLogical) {
					std::vector<size_t> result;
            
					// 1. Encontrar dónde están el Ancla y el Clic visualmente
					size_t anchorVisual = m_module.m_items.GetVisualIndex(anchorLogical);
					size_t currentVisual = clickedVisualIndex;

					// 2. Extraer todos los índices lógicos en ese rango visual
					size_t start = (std::min)(anchorVisual, currentVisual);
					size_t end = (std::max)(anchorVisual, currentVisual);
            
					for (size_t i = start; i <= end; ++i) {
						result.push_back(m_module.m_items.GetLogicalIndex(i));
					}
					return result;
				};
				
				if (m_module.m_selectionController.Select(clickedLogicalIndex, isCtrl, isShift, rangeResolver))
				{
					// Disparar evento a los usuarios de Berta
					//GUI::InvokeEvent(m_module.m_events->Selected, { clickedLogicalIndex });
					GUI::MarkAsNeedUpdate(m_module.m_window);
				}
			}
			else
			{
				// Clic en el espacio vacío: Iniciar recuadro de selección (Lasso) si lo deseas
				m_module.m_lassoSelection.Start(args.Position);
				m_module.m_selectionController.SaveSnapshot();
				GUI::Capture(m_module.m_window);
			}
			
			/*m_module.m_pressedArea = m_module.m_hoveredArea;
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
			else if (m_module.m_pressedArea == InteractionArea::HeaderSplitter || m_module.m_pressedArea == InteractionArea::Header)
			{
				m_module.m_headers.OnMouseDown(args, m_module.m_scrollableView->GetVisibleRect().X);
			}

			if (needUpdate)
			{
				GUI::MarkAsNeedUpdate(m_module.m_window);
			}*/
		}

		void Reactor::MouseMove(Graphics& graphics, const ArgMouse& args)
		{
			auto scrollOffset = m_module.m_scrollableView->GetVisibleRect();

			// 1. DELEGACIÓN AL HEADER (Cambio de cursores, arrastre de columnas)
			if (m_module.m_headers.OnMouseMove(args, scrollOffset.X))
			{
				// Si el usuario está arrastrando una columna, el ancho total cambia.
				// Hay que avisarle al ScrollableView para que actualice la barra horizontal.
				if (m_module.m_headers.IsResizing())
				{
					m_module.UpdateScrollData();
				}
				return;
			}

			// 2. DELEGACIÓN AL LASSO SELECTION (Si el usuario está arrastrando el recuadro)
			if (m_module.m_lassoSelection.IsActive())
			{
				if (m_module.m_lassoSelection.Update(args.Position))
				{
					m_module.ProcessLassoIntersection();
					GUI::MarkAsNeedUpdate(m_module.m_window);
				}
				return;
			}
			

			int headerHeight = static_cast<int>(m_module.m_appearance->HeadersHeight);

			if (args.Position.Y > headerHeight)
			{
				int absoluteY = args.Position.Y - headerHeight + scrollOffset.Y;
				int hoveredIndex = absoluteY / m_module.m_appearance->ListItemHeight;

				if (hoveredIndex >= 0 && hoveredIndex < m_module.m_items.GetCount())
				{
					if (m_module.m_hoveredIndex != hoveredIndex)
					{
						m_module.m_hoveredIndex = hoveredIndex;
						GUI::MarkAsNeedUpdate(m_module.m_window); // Repintar para mostrar el efecto Hover
					}
				}
				else if (m_module.m_hoveredIndex != -1)
				{
					m_module.m_hoveredIndex = -1; // Ratón en área vacía
					GUI::MarkAsNeedUpdate(m_module.m_window);
				}
			}
			
			/*auto hoveredArea = args.ButtonState.NoButtonsPressed() ? m_module.DetermineHoverArea(args.Position) : m_module.m_pressedArea;
			bool needUpdate = false;
			auto scrollOffset = m_module.m_scrollableView->GetScrollOffset();
			
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
						const auto& header = m_module.m_headers.m_headers[headerIndex];
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
						m_module.DrawHeaderItem(draggingBox, { 0,0,columnRect.Width ,columnRect.Height }, header.Text, false, textRect, m_module.m_appearance->SelectionHighlightColor);
					
						draggingBox.Flush();
					}
					m_module.m_headers.m_mouseDraggingPosition = args.Position.X;
					m_module.m_headers.m_isDragging = true;

					auto mousePositionX = args.Position.X + scrollOffset.X - (int)m_module.m_viewport.m_columnOffsetStartOff;
					auto targetHeaderIndex = m_module.GetHeaderAtMousePosition(args.Position, false);

					if (targetHeaderIndex != -1)
					{
						const auto& headerIndex = m_module.m_headers.m_sorted[targetHeaderIndex];
						const auto& headerItem = m_module.m_headers.m_headers[headerIndex];
						auto headerPosX = m_module.m_window->ToScale(headerItem.m_bounds.X);
						auto headerWidth = (int)m_module.m_window->ToScale(headerItem.m_bounds.Width);
						if (targetHeaderIndex == 0 && m_module.m_list.m_drawImages)
						{
							headerWidth += (int)(listItemIconSize + listItemIconMargin * 2u);
						}
						if (targetHeaderIndex > 0 && m_module.m_list.m_drawImages)
						{
							headerPosX += static_cast<int>(listItemIconSize + listItemIconMargin * 2u);
						}
						auto headerHalfWidth = headerWidth >> 1;
						if (mousePositionX >= headerPosX + headerHalfWidth && mousePositionX <= headerPosX + headerWidth)
						{
							targetHeaderIndex++;
						}
					}
					else if (mousePositionX <= static_cast<int>(m_module.m_viewport.m_columnOffsetStartOff))
					{
						targetHeaderIndex = 0;
					}
					else
					{
						targetHeaderIndex = static_cast<int>(m_module.m_headers.m_headers.size());
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

					auto positionY = args.Position.Y - m_module.m_viewport.m_backgroundRect.Y + scrollOffset.Y;
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
					logicalPosition -= scrollOffset;
					m_module.m_mouseSelection.m_endPosition = logicalPosition;

					Point startPoint, endPoint;
					Size boxSize;
					m_module.CalculateSelectionBox(startPoint, endPoint, boxSize);

					Rectangle visibleRect = m_module.m_scrollableView->GetVisibleRect();
					size_t startIndex = visibleRect.Y / m_module.m_viewport.m_itemHeightWithMargin;
					size_t endIndex = std::min<size_t>(m_module.m_list.m_items.size(), static_cast<size_t>((visibleRect.Y + visibleRect.Height) / m_module.m_viewport.m_itemHeightWithMargin) + 1);
			
					needUpdate |= (boxSize.Width > 0 && boxSize.Height > 0);
					if (boxSize.Width > 0 && boxSize.Height > 0)
					{
						Rectangle selectionRect{ startPoint.X + scrollOffset.X, startPoint.Y + scrollOffset.Y * 2 - m_module.m_viewport.m_backgroundRect.Y, boxSize.Width, boxSize.Height };

						for (size_t i = startIndex; i < endIndex; i++)
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
			}*/
		}

		void Reactor::MouseUp(Graphics& graphics, const ArgMouse& args)
		{
			// 1. Liberar Header (Si estaba redimensionando)
			if (m_module.m_headers.OnMouseUp(args))
			{
				return;
			}

			// 2. Liberar Lasso Selection (Si estaba seleccionando)
			/*
			if (m_module.m_lassoSelection.IsActive())
			{
				m_module.m_lassoSelection.End();
				GUI::ReleaseMouse(m_control->Handle());
				GUI::MarkAsNeedUpdate(m_module.m_window);
				return;
			}
			*/
			
			/*bool needUpdate = false;

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
			*/
		}

		void Reactor::MouseLeave(Graphics& graphics, const ArgMouse& args)
		{
			// Restaurar el cursor a la normalidad si se quedó atascado en el Header
			m_module.m_headers.OnMouseLeave();

			// Quitar la iluminación del ítem
			if (m_module.m_hoveredIndex != -1)
			{
				m_module.m_hoveredIndex = -1;
				GUI::MarkAsNeedUpdate(m_module.m_window);
			}
			
			/*if (args.ButtonState.NoButtonsPressed() && m_module.m_hoveredArea == InteractionArea::HeaderSplitter)
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
			*/
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
			m_module.m_shiftPressed = m_module.m_shiftPressed || args.Key == KeyboardKey::Shift;
			m_module.m_ctrlPressed = m_module.m_ctrlPressed || args.Key == KeyboardKey::Control;

			/*bool needUpdate = false;
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
			*/
		}

		void Reactor::KeyReleased(Graphics& graphics, const ArgKeyboard& args)
		{
			if (args.Key == KeyboardKey::Shift) m_module.m_shiftPressed = false;
			if (args.Key == KeyboardKey::Control) m_module.m_ctrlPressed = false;
		}

		void HeaderController::Init(Window* owner)
		{
			m_owner = owner;
		}

		void HeaderController::Append(const std::string& name, uint32_t width)
		{
			m_headers.emplace_back(name, std::max<uint32_t>(width, LISTBOX_MIN_HEADER_WIDTH));
		}

		void HeaderController::Clear()
		{
			m_headers.clear();
			m_resizeInteraction = {};
		}

		uint32_t HeaderController::GetTotalWidth() const
		{
			uint32_t total = 0;
			for (const auto& h : m_headers)
			{
				total += h.Width;
			}
			return total;
		}

		void HeaderController::Draw(Graphics& graphics, const Rectangle& visibleRect, int xOffset)
		{
			auto appearance = reinterpret_cast<Appearance*>(m_owner->Appearance.get());
			int headerHeight = appearance->HeadersHeight;
			int currentX = -visibleRect.X; // Aplicar offset del scroll horizontal

			// Dibujar el fondo base de toda la franja de cabeceras
			Rectangle fullHeaderRect = { 0, 0, m_owner->ClientSize.Width, static_cast<uint32_t>(headerHeight) };
			graphics.DrawRectangle(fullHeaderRect, appearance->Background, true);

			for (size_t i = 0; i < m_headers.size(); ++i)
			{
				const auto& header = m_headers[i];
            
				// Clipping manual: solo dibujamos si está dentro de la ventana
				if (currentX + static_cast<int>(header.Width) > 0 && currentX < m_owner->ClientSize.Width)
				{
					Rectangle headerRect = { currentX, 0, header.Width, static_cast<uint32_t>(headerHeight) };
                
					// Texto de la cabecera (con padding izquierdo)
					Point textPos = { currentX + 6, (int)(headerHeight - graphics.GetTextExtent().Height) / 2 };
					graphics.DrawString(textPos, header.Text, appearance->Foreground);

					// Dibujar el separador vertical derecho
					Point p1 = { currentX + static_cast<int>(header.Width) - 1, 4 };
					Point p2 = { currentX + static_cast<int>(header.Width) - 1, headerHeight - 4 };
					graphics.DrawLine(p1, p2, appearance->BoxBorderColor);
				}
            
				currentX += header.Width;
			}

			// Línea horizontal inferior que separa las cabeceras de la lista
			//graphics.DrawLine({0, headerHeight - 1}, {m_owner->ClientSize.Width, headerHeight - 1}, m_appearance->BorderColor);
		}

		bool HeaderController::OnMouseDown(const ArgMouse& args, int xOffset)
		{
			auto appearance = reinterpret_cast<Appearance*>(m_owner->Appearance.get());
			if (args.Position.Y > appearance->HeadersHeight) return false;

			int colIndex = -1;
			if (IsOverDivider(args.Position.X, xOffset, colIndex))
			{
				m_resizeInteraction.m_isResizing = true;
				m_resizeInteraction.m_columnIndex = colIndex;
				m_resizeInteraction.m_startX = args.Position.X;
				m_resizeInteraction.m_startWidth = m_headers[colIndex].Width;
            
				GUI::Capture(m_owner);
				return true;
			}
        
			return false;
		}

		bool HeaderController::OnMouseMove(const ArgMouse& args, int xOffset)
		{
			auto appearance = reinterpret_cast<Appearance*>(m_owner->Appearance.get());
			if (m_resizeInteraction.m_isResizing)
			{
				int deltaX = args.Position.X - m_resizeInteraction.m_startX;
				int newWidth = static_cast<int>(m_resizeInteraction.m_startWidth) + deltaX;
            
				// Aplicar clamp respetando el ancho mínimo
				m_headers[m_resizeInteraction.m_columnIndex].Width = std::max<uint32_t>(LISTBOX_MIN_HEADER_WIDTH, newWidth);
            
				GUI::MarkAsNeedUpdate(m_owner);
				return true;
			}

			// 2. Lógica de Hover (Cambio de Cursor)
			if (args.Position.Y <= appearance->HeadersHeight)
			{
				int dummyIndex;
				bool overDivider = IsOverDivider(args.Position.X, xOffset, dummyIndex);
            
				if (overDivider && !m_resizeInteraction.m_isHoveringDivider)
				{
					//GUI::SetCursor(m_window, CursorType::SizeWE); // Cursor de ajuste horizontal
					m_resizeInteraction.m_isHoveringDivider = true;
				}
				else if (!overDivider && m_resizeInteraction.m_isHoveringDivider)
				{
					//GUI::SetCursor(m_window, CursorType::Arrow);
					m_resizeInteraction.m_isHoveringDivider = false;
				}
				return true; // Estamos sobre las cabeceras, consumimos el evento
			}
			else if (m_resizeInteraction.m_isHoveringDivider)
			{
				// Salimos de la zona de cabeceras hacia abajo
				//GUI::SetCursor(m_window, CursorType::Arrow);
				m_resizeInteraction.m_isHoveringDivider = false;
			}

			return false;
		}

		bool HeaderController::OnMouseUp(const ArgMouse& args)
		{
			if (m_resizeInteraction.m_isResizing)
			{
				m_resizeInteraction.m_isResizing = false;
				//GUI::ReleaseMouse(m_window);
				return true;
			}
			return false;
		}

		bool HeaderController::OnMouseLeave()
		{
			if (m_resizeInteraction.m_isHoveringDivider && !m_resizeInteraction.m_isResizing)
			{
				//GUI::SetCursor(m_window, CursorType::Arrow);
				m_resizeInteraction.m_isHoveringDivider = false;
			}
			return false;
		}

		bool HeaderController::IsOverDivider(int mouseX, int xOffset, int& outIndex) const
		{
			int currentX = -xOffset;
			for (size_t i = 0; i < m_headers.size(); ++i)
			{
				currentX += m_headers[i].Width;
            
				// Zona de tolerancia de 4 píxeles a cada lado del separador
				if (std::abs(mouseX - currentX) <= 4)
				{
					outIndex = static_cast<int>(i);
					return true;
				}
			}
			return false;
		}

		void Reactor::Module::CalculateViewport(ViewportData& viewportData)
		{
			/*
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
			for (size_t i = 0; i < m_headers.m_headers.size(); i++)
			{
				auto headerWidth = m_window->ToScale(m_headers.m_headers[i].m_bounds.Width);
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
			}*/
		}

		void Reactor::Module::Erase(ListBoxItem item)
		{
			/*auto itemPtr = item.m_target;
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
			UpdateScrollData();

			if (index < m_list.m_items.size())
			{
				BuildListItemBounds(index);
			}

			GUI::UpdateWindow(m_window);*/
		}

		void Reactor::Module::Erase(std::vector<ListBoxItem>& items)
		{
			/*if (items.empty())
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
			UpdateScrollData();

			if (minIndex < m_list.m_items.size())
			{
				BuildListItemBounds(minIndex);
			}

			GUI::UpdateWindow(m_window);*/
		}

		void Reactor::Module::EnableMultiselection(bool enabled)
		{
			m_multiselection = enabled;
		}

		void Reactor::Module::BuildHeaderBounds(size_t startIndex)
		{
			/*
			Point offset{ 0,0 };
			if (startIndex > 0)
			{
				const auto& headerIndex = m_headers.m_sorted[startIndex - 1];
				offset.X = m_headers.m_headers[headerIndex].m_bounds.X + (int)m_headers.m_headers[headerIndex].m_bounds.Width;
			}
			for (size_t i = startIndex; i < m_headers.m_headers.size(); i++)
			{
				const auto& headerIndex = m_headers.m_sorted[i];
				m_headers.m_headers[headerIndex].m_bounds.X = offset.X;

				offset.X += m_headers.m_headers[headerIndex].m_bounds.Width;
			}*/
		}

		void Reactor::Module::BuildListItemBounds(size_t startIndex)
		{
			/*
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
			*/
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
				graphics.DrawString({ boxBounds.X, boxBounds.Y + ((int)(boxBounds.Height - textExtent.Height) >> 1) }, str, textColor);
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

		void Reactor::Module::AppendHeader(const std::string& text, uint32_t width)
		{
			/*
			auto startIndex = m_headers.m_headers.size();
			m_headers.m_headers.emplace_back(text, (std::max)(width, LISTBOX_MIN_HEADER_WIDTH));
			m_headers.m_sorted.emplace_back(m_headers.m_sorted.size());

			CalculateViewport(m_viewport); 
			BuildHeaderBounds(startIndex);*/
		}

		ListBoxItem Reactor::Module::Append(const std::string& text)
		{
			/*
			auto startIndex = m_list.m_items.size();
			m_list.m_items.emplace_back(text);
			m_list.m_sortedIndexes.emplace_back(startIndex);

			for (size_t i = 1; i < m_headers.m_headers.size(); i++)
			{
				m_list.m_items.back().m_cells.emplace_back("");
			}
			if (m_headers.m_sortedHeaderIndex != -1)
			{
				size_t selectedHeaderIndex = static_cast<size_t>(m_headers.m_sortedHeaderIndex);

				SortHeader(m_headers.m_sorted[selectedHeaderIndex], m_headers.isAscendingOrdering);
			}

			CalculateViewport(m_viewport);
			BuildListItemBounds(startIndex);
			UpdateScrollData();

			GUI::UpdateWindow(m_window);

			return { &m_list.m_items.back(), this};
			*/
			return {0, nullptr};
		}

		ListBoxItem Reactor::Module::Append(std::initializer_list<std::string> texts)
		{
			/*
			auto startIndex = m_list.m_items.size();

			auto headersCount = m_headers.m_headers.size();
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
			BuildListItemBounds(startIndex);

			return { &m_list.m_items.back(), this };*/
			
			return {0, nullptr};
		}

		ListBoxItem Reactor::Module::At(size_t index)
		{
			auto wrapper = m_items.At(index);
			return ListBoxItem{ 0, &m_items };
		}

		void Reactor::Module::Clear()
		{
			/*
			bool needUpdate = !m_list.m_items.empty();
			m_list.m_items.clear();
			m_list.m_sortedIndexes.clear();

			m_mouseSelection.Clear();
			CalculateViewport(m_viewport);
			UpdateScrollData();

			if (needUpdate)
			{
				GUI::UpdateWindow(m_window);
			}
			*/
		}

		void Reactor::Module::ClearHeaders()
		{
			/*
			bool needUpdate = !m_headers.m_headers.empty();
			m_headers.m_headers.clear();
			m_headers.m_sorted.clear();
			m_list.m_items.clear();

			m_headers.m_selectedIndex = -1;
			m_headers.m_draggingTargetIndex = -1;

			if (needUpdate)
			{
				GUI::UpdateWindow(m_window);
			}*/
		}

		void Reactor::Module::UpdateScrollData()
		{
			if (!m_scrollableView)
			{
				return;
			}
			auto headerHeight = m_window->ToScale(m_appearance->HeadersHeight);
			m_scrollableView->SetViewSize(m_window->ClientSize);
			m_scrollableView->SetViewPadding(headerHeight, 0, 0, 0);
			
			Size contentSize;
        
			contentSize.Width = m_viewport.m_columnOffsetStartOff;
			for (const auto& header : m_headers.GetHeaders())
			{
				auto headerWidth = m_window->ToScale(header.Width);
				contentSize.Width += headerWidth;
			}

			contentSize.Height = static_cast<uint32_t>(m_items.GetCount()) * (m_viewport.m_itemHeightWithMargin) + headerHeight;

			/*if (m_list.m_drawImages)
			{
				auto listItemIconSize = m_window->ToScale(m_appearance->ListItemIconSize);
				auto listItemIconMargin = m_window->ToScale(m_appearance->ListItemIconMargin);
				contentSize.Width += listItemIconSize + listItemIconMargin * 2u;
			}*/
			m_scrollableView->SetContentSize(contentSize);
		}

		void Reactor::Module::DrawHeaders(Graphics& graphics)
		{
			/*
			auto headerHeight = m_window->ToScale(m_appearance->HeadersHeight);
			auto leftMarginTextHeader = m_window->ToScale(5u);
			auto listItemIconSize = m_window->ToScale(m_appearance->ListItemIconSize);
			auto listItemIconMargin = m_window->ToScale(m_appearance->ListItemIconMargin);

			int sortedHeaderMargin = m_window->ToScale(4);
			int arrowSortedHeaderSize = m_window->ToScale(6);
			auto scrollOffset = m_scrollableView->GetScrollOffset();
			graphics.DrawGradientFill({ 0,0, m_window->ClientSize.Width, headerHeight }, m_appearance->ButtonHighlightBackground, m_appearance->ButtonBackground);
			graphics.DrawLine({ m_viewport.m_backgroundRect.X + (int)m_viewport.m_columnOffsetStartOff - scrollOffset.X - 1, 0 }, { m_viewport.m_backgroundRect.X + (int)m_viewport.m_columnOffsetStartOff - scrollOffset.X - 1, (int)headerHeight - 1 }, m_appearance->BoxBorderColor);
		
			Point headerOffset{ m_viewport.m_backgroundRect.X + (int)m_viewport.m_columnOffsetStartOff - scrollOffset.X, m_viewport.m_backgroundRect.Y };

			for (size_t i = 0; i < m_headers.m_headers.size(); ++i)
			{
				const auto& headerIndex = m_headers.m_sorted[i];
				const auto& header = m_headers.m_headers[headerIndex];
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
				DrawHeaderItem(graphics, columnRect, header.Text, isHovered, textRect, m_appearance->Foreground);

				if (isDragging)
				{
					int lineWidth = m_window->ToScale(2);
					auto targetHeaderPosition = 0;
					if (m_headers.m_draggingTargetIndex < m_headers.m_headers.size())
					{
						const auto& headerIndex = m_headers.m_sorted[m_headers.m_draggingTargetIndex];
						targetHeaderPosition = m_window->ToScale(m_headers.m_headers[headerIndex].m_bounds.X);
					}
					else
					{
						const auto& headerIndex = m_headers.m_sorted[m_headers.m_headers.size() - 1];
						const auto& lastHeaderBounds = m_headers.m_headers[headerIndex].m_bounds;
						targetHeaderPosition = m_window->ToScale(lastHeaderBounds.X + lastHeaderBounds.Width);
					}
					targetHeaderPosition += m_viewport.m_backgroundRect.X + (int)m_viewport.m_columnOffsetStartOff - scrollOffset.X;
					if (m_headers.m_draggingTargetIndex != 0 && m_list.m_drawImages)
					{
						targetHeaderPosition += (int)(listItemIconSize + listItemIconMargin * 2u);
					}
					graphics.DrawLine({ targetHeaderPosition, 0 }, { targetHeaderPosition, (int)headerHeight - lineWidth }, static_cast<float>(lineWidth), m_appearance->SelectionHighlightColor);

					Graphics& draggingBox = m_headers.m_draggingBox;
				
					auto headerPosition = m_headers.m_headers[headerIndex].m_bounds.X;
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
			*/
		}

		void Reactor::Module::DrawHeaderItem(Graphics& graphics, const Rectangle& rect, const std::string& name, bool isHovered, const Rectangle& textRect, const Color& textColor)
		{
			if (isHovered)
			{
				graphics.DrawRectangle(rect, m_appearance->HighlightColor, true);
			}

			DrawStringInBox(graphics, name, textRect, textColor);
		}

		void Reactor::Module::DrawList(Graphics& graphics)
		{
			if (m_items.IsEmpty()) return;

			// 1. Obtener contexto geométrico (Gracias al ScrollableView)
			Rectangle visibleRect = m_scrollableView->GetVisibleRect();
			int itemHeight = m_appearance->ListItemHeight;
			int headerHeight = m_appearance->HeadersHeight;

			// 2. Calcular rango visible (Matemática O(1))
			int listVisibleY = std::max<int>(0, visibleRect.Y - static_cast<int>(headerHeight));
			int startIndex = listVisibleY / itemHeight;
			int endIndex = std::min<int>((int)m_items.GetCount(), (listVisibleY + (int)visibleRect.Height) / itemHeight + 1);

			// 3. Iterar y dibujar
			int currentY = (startIndex * itemHeight) + headerHeight - visibleRect.Y;
    
			for (int i = startIndex; i < endIndex; ++i)
			{
				Rectangle rowRect = m_scrollableView->GetClientArea();
				rowRect.Y = currentY;
				rowRect.Height = itemHeight;

				// Ajuste por scroll horizontal
				rowRect.X -= visibleRect.X; 
        
				// DELEGACIÓN: Métodos pequeños y específicos
				DrawRowBackground(graphics, i, rowRect);
				DrawRowContent(graphics, i, rowRect);

				currentY += itemHeight;
			}
			
			/*
			bool enabled = true;
			auto listItemIconSize = m_window->ToScale(m_appearance->ListItemIconSize);
			auto listItemIconMargin = m_window->ToScale(m_appearance->ListItemIconMargin);
			auto scrollOffset = m_scrollableView->GetScrollOffset();
			Point listOffset{ m_viewport.m_backgroundRect.X + static_cast<int>(m_viewport.m_columnOffsetStartOff) - scrollOffset.X, m_viewport.m_backgroundRect.Y - scrollOffset.Y };
			
			Rectangle visibleRect = m_scrollableView->GetVisibleRect();
			
			auto& itemHeight = m_viewport.m_itemHeight;
			auto& itemHeightWithMargin = m_viewport.m_itemHeightWithMargin;
			auto leftMarginListItemText = m_window->ToScale(3u);
			auto headerHeight = m_window->ToScale(m_appearance->HeadersHeight);
			int listVisibleY = std::max<int>(0, visibleRect.Y - static_cast<int>(headerHeight));
			
			size_t startIndex = listVisibleY / itemHeightWithMargin;
			size_t endIndex = std::min<size_t>(m_list.m_items.size(), static_cast<size_t>((visibleRect.Y + visibleRect.Height) / itemHeightWithMargin) + 1);
			
			Rectangle itemRect = m_scrollableView->GetClientArea();
			itemRect.Height = itemHeight;
			itemRect.Width = m_viewport.m_contentSize.Width - m_viewport.m_columnOffsetStartOff;
			
			for (size_t i = startIndex; i < endIndex; i++)
			{
				auto absoluteIndex = m_list.m_sortedIndexes[i];
				auto& item = m_list.m_items[absoluteIndex];
				itemRect.Y = (i * itemHeightWithMargin) - visibleRect.Y + headerHeight + m_viewport.m_innerMargin;
				itemRect.X = static_cast<int>(m_viewport.m_columnOffsetStartOff) + m_scrollableView->GetClientArea().X - visibleRect.X;
				int cellOffset = 0;

				bool isLastSelected = &item == m_mouseSelection.m_selectedItem;
				bool isHovered = m_mouseSelection.m_hoveredItem == &item;
				bool isSelected = item.m_isSelected;
				//Rectangle itemRect{ listOffset.X, listOffset.Y + (int)m_viewport.m_innerMargin + (int)(itemHeightWithMargin * i), m_viewport.m_contentSize.Width - m_viewport.m_columnOffsetStartOff, itemHeight };
				
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
					const auto& header = m_headers.m_headers[headerIndex];
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
					if (cellOffset - scrollOffset.X >= static_cast<int>(m_viewport.m_backgroundRect.Width))
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
			*/
		}

		void Reactor::Module::CalculateSelectionBox(Point& startPoint, Point& endPoint, Size& boxSize) const
		{
			/*
			startPoint = {
				(std::min)(m_mouseSelection.m_startPosition.X, m_mouseSelection.m_endPosition.X),
				(std::min)(m_mouseSelection.m_startPosition.Y, m_mouseSelection.m_endPosition.Y)
			};

			endPoint = {
				(std::max)(m_mouseSelection.m_startPosition.X, m_mouseSelection.m_endPosition.X),
				(std::max)(m_mouseSelection.m_startPosition.Y, m_mouseSelection.m_endPosition.Y)
			};

			auto scrollOffset = m_scrollableView->GetScrollOffset();
			startPoint.Y = (std::max)(startPoint.Y, m_viewport.m_backgroundRect.Y - scrollOffset.Y);
			startPoint.X = (std::max)(startPoint.X, m_viewport.m_backgroundRect.X - scrollOffset.X);

			boxSize = { static_cast<uint32_t>(endPoint.X - startPoint.X), static_cast<uint32_t>(endPoint.Y - startPoint.Y) };
			*/
		}

		bool Reactor::Module::SetHoveredListItem(List::Item* index)
		{
			/*
			if (m_mouseSelection.m_hoveredItem != index)
			{
				m_mouseSelection.m_hoveredItem = index;
				return true;
			}
			*/
			return false;
		}

		void Reactor::Module::StopDragOrSortHeader()
		{
			/*
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
			*/
		}

		void Reactor::Module::SortHeader(size_t headerIndex, bool ascending)
		{
			/*
			if (m_list.m_items.empty() || headerIndex >= m_headers.m_headers.size())
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
			*/
		}

		int Reactor::Module::GetListItemIndex(const List::Item* item) const
		{
			/*
			for (size_t i = 0; i < m_list.m_items.size(); i++)
			{
				if (item == &m_list.m_items[m_list.m_sortedIndexes[i]])
				{
					return static_cast<int>(i);
				}
			}*/
			return -1;
		}

		void Reactor::Module::InitScrollableView()
		{
			m_scrollableView = std::make_unique<ScrollableView>(m_window);
        
			// Configuramos el paso del scroll (ej. saltar de a 1 ítem)
			m_scrollableView->SetScrollStep(m_appearance->ListItemHeight, 20);

			// Cuando el ScrollableView detecta un cambio, repintamos el ListBox
			m_scrollableView->SetOnScrollChange([this]()
				{
					GUI::MarkAsNeedUpdate(m_window);
				});
		}

		void Reactor::Module::ProcessLassoIntersection()
		{
			if (!m_lassoSelection.IsActive() || m_items.IsEmpty()) return;

			Rectangle lasso = m_lassoSelection.GetRect();
			int itemHeight = m_appearance->ListItemHeight;
			int scrollY = m_scrollableView->GetScrollOffset().Y;

			int absoluteTop = (std::max)(0, lasso.Y - static_cast<int>(m_appearance->HeadersHeight) + scrollY);
			int absoluteBottom = (lasso.Y + lasso.Height) - m_appearance->HeadersHeight + scrollY;

			if (absoluteBottom < 0) return;

			size_t startIndex = absoluteTop / itemHeight;
			size_t endIndex = absoluteBottom / itemHeight;

			std::vector<size_t> lassoedIndices;
			for (size_t i = startIndex; i <= endIndex; ++i)
			{
				lassoedIndices.push_back(i);
			}

			bool isCtrl = m_ctrlPressed;

			// DELEGACIÓN: Actualizamos el rango basado en la foto guardada
			if (m_selectionController.ApplyLassoSelection(lassoedIndices, isCtrl))
			{
				GUI::MarkAsNeedUpdate(m_window);
			}
		}

		void Reactor::Module::UpdateSelectionRange(int startIndex, int endIndex)
		{
			// 1. Restaurar todos los ítems al estado "Pre-Lasso"
			for (size_t i = 0; i < m_items.GetCount(); ++i)
			{
				bool wasSelected = m_preLassoSelection.find(i) != m_preLassoSelection.end();
				SetItemSelected(i, wasSelected); // Tu método para cambiar selección
			}

			// 2. Aplicar la selección ACTUAL del recuadro
			// (Si apretó Ctrl, invertimos. Si no, forzamos selección)
			bool isCtrlPressed = m_ctrlPressed;

			for (int i = startIndex; i <= endIndex; ++i)
			{
				if (isCtrlPressed)
				{
					// Ctrl: Intercambia el estado previo
					bool wasSelected = m_preLassoSelection.find(i) != m_preLassoSelection.end();
					SetItemSelected(i, !wasSelected);
				}
				else
				{
					// Normal: Fuerza a que esté seleccionado
					SetItemSelected(i, true);
				}
			}
		}

		void Reactor::Module::SetItemSelected(size_t index, bool selected)
		{
			// 1. Verificación de seguridad (Bounds check)
			// Asumiendo que m_items es tu ItemCollection o std::vector
			if (index >= m_items.GetCount()) 
			{
				return;
			}

			// 2. Delegamos la acción al controlador
			m_selectionController.SetSelected(index, selected);

			// Nota: No llamamos a GUI::MarkAsNeedUpdate(m_window) aquí adentro 
			// porque este método suele llamarse en bucles (ej. durante el Lasso Selection).
			// Es mucho más eficiente repintar la ventana una sola vez al terminar el bucle.
		}

		void Reactor::Module::DrawRowBackground(Graphics& graphics, int visualIndex, const Rectangle& rowRect)
		{
			Color bgColor = m_appearance->BoxBackground;
			size_t logicalIndex = m_items.GetLogicalIndex(visualIndex);
			bool isSelected = m_selectionController.IsSelected(logicalIndex); 
			bool isHovered = (visualIndex == m_hoveredIndex);
			
			if (isSelected)
			{
				bgColor = m_appearance->SelectionHighlightColor;
			}
			else if (isHovered)
			{
				bgColor = m_appearance->HighlightColor;
			}

			// Solo dibujamos si es distinto al fondo base para ahorrar llamadas a GPU
			if (bgColor != m_appearance->BoxBackground)
			{
				// Nota: Usamos el ancho total del contenido para que el color de selección no se corte al scrollear
				Rectangle bgRect = rowRect;
				bgRect.Width = std::max<uint32_t>(rowRect.Width, m_headers.GetTotalWidth()); 
				graphics.DrawRectangle(bgRect, bgColor, true);
			}
		}

		void Reactor::Module::DrawRowContent(Graphics& graphics, int visualIndex, const Rectangle& rowRect)
		{
			size_t logicalIndex = m_items.GetLogicalIndex(visualIndex);
			const auto item = m_items.At(logicalIndex);
			const auto& headers = m_headers.GetHeaders();
    
			int currentX = rowRect.X;

			// Iteramos por columnas (Celdas)
			for (size_t col = 0; col < headers.size(); ++col)
			{
				uint32_t colWidth = headers[col].Width;
        
				// Optimización: "Clipping" manual horizontal (no dibujar lo que no se ve)
				if (currentX + (int)colWidth > 0 && currentX < m_window->ClientSize.Width)
				{
					Rectangle cellRect = { currentX, rowRect.Y, colWidth, rowRect.Height };
            
					// Obtenemos el texto de forma segura
					std::string text = (col < item->m_cells.size()) ? item->m_cells[col].m_text : "";
            
					DrawCell(graphics, cellRect, text, IsSelected(visualIndex));
				}
				currentX += colWidth;
			}
		}

		void Reactor::Module::DrawCell(Graphics& graphics, const Rectangle& rect, const std::string& text, bool isRowSelected)
		{
			// Padding interno de la celda
			Rectangle textRect = rect;
			textRect.X += 4; 
			textRect.Width -= 8;

			Color textColor = isRowSelected ? m_appearance->HighlightTextColor : m_appearance->Foreground;
    
			// Aquí podrías añadir lógica de elipsis (...) si el texto es muy largo
			graphics.DrawString(textRect.Position(), text, textColor);
		}

		bool Reactor::Module::IsSelected(int index) const
		{
			return false;
		}

		bool Reactor::Module::HandleMultiSelection(List::Item* item, const ArgMouse& args)
		{
			/*
			bool needUpdate = false;
			const auto absoluteIndex = GetListItemIndex(item);
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
			*/
			return false;
		}

		bool Reactor::Module::UpdateSingleSelection(List::Item* item)
		{
			/*bool needUpdate = m_mouseSelection.m_selectedItem != item || !item->m_isSelected;
			if (needUpdate)
			{
				ClearSingleSelection();
				SelectItem(item);
			}
			return needUpdate;*/
			return false;
		}

		void Reactor::Module::ToggleItemSelection(List::Item* item)
		{
			/*item->m_isSelected = !item->m_isSelected;

			if (item->m_isSelected)
			{
				m_mouseSelection.m_selections.push_back(item);
			}
			else
			{
				auto it = std::remove(m_mouseSelection.m_selections.begin(), m_mouseSelection.m_selections.end(), item);
				m_mouseSelection.m_selections.erase(it, m_mouseSelection.m_selections.end());
			}*/
		}

		void Reactor::Module::StartSelectionRectangle(const Point& mousePosition)
		{
			/*
			auto logicalPosition = mousePosition;
			logicalPosition -= m_scrollableView->GetScrollOffset();
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

			GUI::Capture(m_window);*/
		}

		bool Reactor::Module::ClearSelectionIfNeeded()
		{
			/*if (!m_ctrlPressed && !m_shiftPressed)
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
			}*/
			return false;
		}

		bool Reactor::Module::ClearSingleSelection()
		{
			/*if (m_mouseSelection.m_selectedItem)
			{
				m_mouseSelection.m_selectedItem->m_isSelected = false;
				m_mouseSelection.m_selections.clear();
				m_mouseSelection.m_selectedItem = nullptr;
				return true;
			}*/
			return false;
		}

		std::vector<ListBoxItem> Reactor::Module::GetSelectedItems()
		{
			std::vector<ListBoxItem> selections;
			/*std::vector<size_t> indexes;

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
			}*/
			return selections;
		}

		void Reactor::Module::ClearSelection()
		{
			/*
			for (size_t i = 0; i < m_mouseSelection.m_selections.size(); i++)
			{
				m_mouseSelection.m_selections[i]->m_isSelected = false;
			}
			m_mouseSelection.m_selections.clear();*/
		}

		void Reactor::Module::SelectItem(List::Item* item)
		{
			/*item->m_isSelected = true;
			m_mouseSelection.m_selections.push_back(item);
			m_mouseSelection.m_selectedItem = item;*/
		}

		bool Reactor::Module::EnsureVisibility(int itemIndex)
		{
			/*
			if (itemIndex >= m_list.m_items.size() || !m_scrollableView)
			{
				return false;
			}

			auto itemHeight = static_cast<int>(m_viewport.m_itemHeightWithMargin);
			int headerHeight = static_cast<int>(m_appearance->HeadersHeight);

			Rectangle targetBounds;
			targetBounds.X = 0; 
			targetBounds.Y = (itemIndex * itemHeight) + headerHeight;
			targetBounds.Width = 10;
			targetBounds.Height = itemHeight;

			bool scrollChanged = m_scrollableView->EnsureVisibility(targetBounds);
			if (scrollChanged)
			{
				GUI::MarkAsNeedUpdate(m_window);
			}
			return scrollChanged;*/
			return false;
		}

		void Reactor::Module::PerformRangeSelection(List::Item* pressedItem)
		{
			/*
			auto pressedItemIndex = GetListItemIndex(pressedItem);
			auto pivotIndex = GetListItemIndex(m_mouseSelection.m_pivotItem);
			int minIndex = (std::min)(pivotIndex, pressedItemIndex);
			int maxIndex = (std::max)(pivotIndex, pressedItemIndex);

			for (int i = minIndex; i <= maxIndex; ++i)
			{
				auto currentItem = &m_list.m_items[m_list.m_sortedIndexes[i]];
				currentItem->m_isSelected = true;
				m_mouseSelection.m_selections.push_back(currentItem);
			}*/
		}

/*
		bool Reactor::MouseSelection::IsAlreadySelected(List::Item* index) const
		{
			return std::find(m_alreadySelected.begin(), m_alreadySelected.end(), index) != m_alreadySelected.end();
		}

		bool Reactor::MouseSelection::IsSelected(List::Item* item) const
		{
			return std::find(m_selections.begin(), m_selections.end(), item) != m_selections.end();
		}

		void Reactor::MouseSelection::Select(List::Item* item)
		{
			m_selections.push_back(item);
		}

		void Reactor::MouseSelection::Deselect(List::Item* item)
		{
			auto it = std::find(m_selections.begin(), m_selections.end(), item);
			if (it != m_selections.end())
			{
				m_selections.erase(it);
			}
		}

		void Reactor::MouseSelection::Clear()
		{
			m_selections.clear();
			m_alreadySelected.clear();

			m_selectedItem = nullptr;
			m_pivotItem = nullptr;
			m_hoveredItem = nullptr;
		}

		void Reactor::MouseSelection::ClearReferences(List::Item* item)
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
		}*/

		int Reactor::Module::GetHeaderAtMousePosition(const Point& mousePosition, bool splitter) const
		{
			/*auto listItemIconSize = m_window->ToScale(m_appearance->ListItemIconSize);
			auto listItemIconMargin = m_window->ToScale(m_appearance->ListItemIconMargin);
			Point headerOffset{ m_viewport.m_backgroundRect.X + (int)m_viewport.m_columnOffsetStartOff - m_scrollableView->GetScrollOffset().X, 0 };
		
			auto splitterThreshold = m_window->ToScale(3);
			for (size_t i = 0; i < m_headers.m_headers.size(); i++)
			{
				const auto& headerIndex = m_headers.m_sorted[i];
				const auto& header = m_headers.m_headers[headerIndex];
				auto headerWidth = m_window->ToScale(header.m_bounds.Width);
				if (i == 0 && m_list.m_drawImages)
				{
					headerWidth += listItemIconSize + listItemIconMargin * 2u;
				}
				const auto headerWidthInt = static_cast<int>(headerWidth);
				if (headerOffset.X + headerWidthInt < -splitterThreshold || headerOffset.X - splitterThreshold >= (int)m_viewport.m_backgroundRect.Width)
				{
					headerOffset.X += headerWidthInt;
					continue;
				}

				if (splitter && mousePosition.X >= headerOffset.X + headerWidthInt - splitterThreshold &&
					mousePosition.X <= headerOffset.X + headerWidthInt + splitterThreshold)
				{
					return static_cast<int>(i);
				}
				if (!splitter && mousePosition.X >= headerOffset.X &&
					mousePosition.X < headerOffset.X + headerWidthInt)
				{
					return static_cast<int>(i);
				}

				headerOffset.X += headerWidthInt;
			}*/
			return -1;
		}

		void Reactor::Module::StartHeadersSizing(const Point& mousePosition)
		{
			/*
			 *GUI::Capture(m_window);
			m_headers.m_selectedIndex = GetHeaderAtMousePosition(mousePosition, true);
			int iconWidth = 0;
			if (m_list.m_drawImages && m_headers.m_selectedIndex == 0)
			{
				auto listItemIconSize = m_window->ToScale(m_appearance->ListItemIconSize);
				auto listItemIconMargin = m_window->ToScale(m_appearance->ListItemIconMargin);
				iconWidth += static_cast<int>(listItemIconSize + listItemIconMargin * 2u);
			}
			const auto& headerIndex = m_headers.m_sorted[m_headers.m_selectedIndex];
			m_headers.m_mouseDownOffset = m_scrollableView->GetScrollOffset().X + mousePosition.X - static_cast<int>(m_window->ToScale(
				m_headers.m_headers[headerIndex].m_bounds.X + m_headers.m_headers[headerIndex].m_bounds.Width + iconWidth));
				*/
		}

		void Reactor::Module::UpdateHeadersSize(const Point& mousePosition)
		{
			/*
			const auto& headerIndex = m_headers.m_sorted[m_headers.m_selectedIndex];
			auto& headerBounds = m_headers.m_headers[headerIndex].m_bounds;
			auto newWidth = m_window->ToDownwardScale(m_scrollableView->GetScrollOffset().X + mousePosition.X - m_headers.m_mouseDownOffset - m_window->ToScale(headerBounds.X));
			if (m_list.m_drawImages && m_headers.m_selectedIndex == 0)
			{
				int iconWidth = 0;
				auto listItemIconSize = m_window->ToScale(m_appearance->ListItemIconSize);
				auto listItemIconMargin = m_window->ToScale(m_appearance->ListItemIconMargin);
				iconWidth += listItemIconSize + listItemIconMargin * 2u;
				newWidth -= iconWidth;
			}
			headerBounds.Width = (std::max)(LISTBOX_MIN_HEADER_WIDTH, static_cast<uint32_t>((std::max)(0, newWidth)));
			CalculateViewport(m_viewport);
		
			UpdateScrollData();
			BuildHeaderBounds(m_headers.m_selectedIndex);*/
		}

		void Reactor::Module::StopHeadersSizing()
		{
			/*
			GUI::ReleaseCapture(m_window);
			m_headers.m_selectedIndex = -1;*/
		}

		void Reactor::Module::StartSelectingHeader(const Point& mousePosition)
		{
			/*
			GUI::Capture(m_window);
			m_headers.m_isDragging = false;
			m_headers.m_selectedIndex = GetHeaderAtMousePosition(mousePosition, false);
			const auto& headerIndex = m_headers.m_sorted[m_headers.m_selectedIndex];
			m_headers.m_mouseDownOffset = mousePosition.X - m_window->ToScale(m_headers.m_headers[headerIndex].m_bounds.X) - m_viewport.m_backgroundRect.X - static_cast<int>(m_viewport.m_columnOffsetStartOff) + m_scrollableView->GetScrollOffset().X;
			*/
		}

		void ListBoxItem::SetIcon(const Image& image) const
		{
			/*
			m_target->m_icon = image;
			if (image)
			{
				m_module->m_list.m_drawImages = true;
				m_module->BuildHeaderBounds();
			}*/
		}

		void ListBoxItem::SetText(size_t columnIndex, const std::string& text) const
		{
			auto target = m_collection->GetItemSafely(m_logicalIndex);
			if (!target || columnIndex >= target->m_cells.size())
				return;
			
			target->m_cells[columnIndex].m_text = text;
			m_collection->NotifyItemModified();
			/*
			if (columnIndex >= m_module->m_headers.m_headers.size())
			{
				return;
			}

			bool needUpdate = m_target->m_cells[columnIndex].m_text != text;

			m_target->m_cells[columnIndex].m_text = text;
			if (m_module->m_headers.m_sortedHeaderIndex == static_cast<int>(columnIndex))
			{
				m_module->SortHeader(m_module->m_headers.m_sorted[m_module->m_headers.m_sortedHeaderIndex], m_module->m_headers.isAscendingOrdering);
			}

			if (needUpdate)
			{
				GUI::UpdateWindow(m_module->m_window);
			}
			*/
		}

		std::string ListBoxItem::GetText(size_t columnIndex) const
		{
			auto target = m_collection->GetItemSafely(m_logicalIndex);
			if (!target || columnIndex >= target->m_cells.size())
				return {};

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

	ListBox::ListBoxItem ListBox::At(size_t index)
	{
		return GetReactor().GetModule().At(index);
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
