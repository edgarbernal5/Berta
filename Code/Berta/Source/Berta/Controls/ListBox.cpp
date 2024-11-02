/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#include "btpch.h"
#include "ListBox.h"

#include "Berta/GUI/Interface.h"
#include "Berta/GUI/EnumTypes.h"

namespace Berta
{
	void ListBoxReactor::Init(ControlBase& control)
	{
		m_control = &control;
		m_module.m_window = control.Handle();

		m_module.m_appearance = reinterpret_cast<ListBoxAppearance*>(control.Handle()->Appearance.get());

		m_module.CalculateViewport(m_module.m_viewport);
		m_module.CalculateVisibleIndices();
	}

	void ListBoxReactor::Update(Graphics& graphics)
	{
		auto enabled = m_control->GetEnabled();
		graphics.DrawRectangle(m_module.m_window->Size.ToRectangle(), m_module.m_window->Appearance->BoxBackground, true);

		DrawList(graphics);
		DrawHeaders(graphics);

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
			startPoint.Y = (std::max)(startPoint.Y, m_module.m_viewport.BackgroundRect.Y - m_module.ScrollOffset.Y);

			Size boxSize{ (uint32_t)(endPoint.X - startPoint.X), (uint32_t)(endPoint.Y - startPoint.Y) };
			Color blendColor = m_module.m_window->Appearance->HighlightColor;
			Graphics selectionBox(boxSize);
			selectionBox.DrawRectangle(blendColor, true);
			selectionBox.DrawRectangle(m_module.m_window->Appearance->BoxBorderColor, false);

			Rectangle blendRect{ startPoint.X + m_module.ScrollOffset.X, startPoint.Y + m_module.ScrollOffset.Y, boxSize.Width, boxSize.Height };
			graphics.Blend(blendRect, selectionBox, { 0,0 }, 0.5f);
		}

		if (m_module.m_viewport.NeedHorizontalScroll && m_module.m_viewport.NeedVerticalScroll)
		{
			auto scrollSize = m_module.m_window->ToScale(m_module.m_window->Appearance->ScrollBarSize);
			graphics.DrawRectangle({ (int)(m_module.m_window->Size.Width - scrollSize) - 1, (int)(m_module.m_window->Size.Height - scrollSize) - 1, scrollSize, scrollSize }, m_module.m_window->Appearance->Background, true);
		}
		graphics.DrawRectangle(m_module.m_window->Size.ToRectangle(), enabled ? m_module.m_window->Appearance->BoxBorderColor : m_module.m_window->Appearance->BoxBorderDisabledColor, false);
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
		m_module.m_mouseDownPosition = args.Position;
		bool needUpdate = false;

		if (m_module.m_pressedArea == InteractionArea::List)
		{
			m_module.m_mouseSelection.m_pressedIndex = m_module.m_mouseSelection.m_hoveredIndex;

			if (m_module.m_multiselection)
			{
				needUpdate = m_module.HandleMultiSelection(m_module.m_mouseSelection.m_hoveredIndex, args);
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

		if (needUpdate)
		{
			Update(graphics);
			GUI::MarkAsUpdated(m_module.m_window);
		}
	}

	void ListBoxReactor::MouseMove(Graphics& graphics, const ArgMouse& args)
	{
		auto hoveredArea = args.ButtonState.NoButtonsPressed() ? m_module.DetermineHoverArea(args.Position) : m_module.m_pressedArea;
		bool needUpdate = hoveredArea != m_module.m_hoveredArea;

		if ((hoveredArea == InteractionArea::Header || hoveredArea == InteractionArea::HeaderSplitter))
		{
			if (args.ButtonState.NoButtonsPressed())
			{
				if (m_module.m_mouseSelection.m_hoveredIndex != -1)
				{
					m_module.m_mouseSelection.m_hoveredIndex = -1;
					needUpdate = true;
				}

				if (hoveredArea == InteractionArea::HeaderSplitter)
				{
					GUI::ChangeCursor(m_module.m_window, Cursor::SizeWE);
				}
			}
			else if (hoveredArea == InteractionArea::HeaderSplitter && args.ButtonState.LeftButton)
			{
				m_module.UpdateHeadersSizing(args.Position);

				needUpdate = true;
			}
		}
		else if (hoveredArea == InteractionArea::List)
		{
			if (args.ButtonState.NoButtonsPressed())
			{
				auto itemHeight = m_module.m_window->ToScale(m_module.m_appearance->ListItemHeight) + m_module.m_viewport.InnerMargin * 2u;

				auto positionY = args.Position.Y - m_module.m_viewport.BackgroundRect.Y + m_module.ScrollOffset.Y;
				int index = positionY / (int)itemHeight;

				if (m_module.m_mouseSelection.m_hoveredIndex != index)
				{
					m_module.m_mouseSelection.m_hoveredIndex = index;
					needUpdate = true;
				}
			}
		}
		else if (hoveredArea == InteractionArea::ListBlank || hoveredArea == InteractionArea::None)
		{
			if (m_module.m_mouseSelection.m_hoveredIndex != -1)
			{
				m_module.m_mouseSelection.m_hoveredIndex = -1;
				needUpdate = true;
			}

			if (m_module.m_mouseSelection.m_started)
			{
				auto logicalPosition = args.Position;
				logicalPosition -= m_module.ScrollOffset;
				m_module.m_mouseSelection.m_endPosition = logicalPosition;

				Point startPoint{
					(std::min)(m_module.m_mouseSelection.m_startPosition.X, m_module.m_mouseSelection.m_endPosition.X),
					(std::min)(m_module.m_mouseSelection.m_startPosition.Y, m_module.m_mouseSelection.m_endPosition.Y)
				};

				Point endPoint{
					(std::max)(m_module.m_mouseSelection.m_startPosition.X, m_module.m_mouseSelection.m_endPosition.X),
					(std::max)(m_module.m_mouseSelection.m_startPosition.Y, m_module.m_mouseSelection.m_endPosition.Y)
				};
				startPoint.Y = (std::max)(startPoint.Y, m_module.m_viewport.BackgroundRect.Y - m_module.ScrollOffset.Y);
				Size boxSize{ (uint32_t)(endPoint.X - startPoint.X), (uint32_t)(endPoint.Y - startPoint.Y) };

				needUpdate |= (boxSize.Width > 0 && boxSize.Height > 0);
				if ((boxSize.Width > 0 && boxSize.Height > 0))
				{
					Rectangle selectionRect{ startPoint.X + m_module.ScrollOffset.X, startPoint.Y + m_module.ScrollOffset.Y * 2 - m_module.m_viewport.BackgroundRect.Y, boxSize.Width, boxSize.Height };

					for (size_t i = m_module.m_viewport.StartingVisibleIndex; i < m_module.m_viewport.EndingVisibleIndex; i++)
					{
						auto& item = m_module.List.Items[i];
						item.Bounds.Y = m_module.m_viewport.ItemHeightWithMargin * i;
						item.Bounds.Width = m_module.m_viewport.ContentSize.Width;
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
		}

		if (args.ButtonState.NoButtonsPressed() && hoveredArea != InteractionArea::HeaderSplitter && m_module.m_hoveredArea == InteractionArea::HeaderSplitter)
		{
			BT_CORE_DEBUG << "  - mo move / cursor default" << std::endl;
			GUI::ChangeCursor(m_module.m_window, Cursor::Default);
		}


		if (hoveredArea == InteractionArea::ListBlank && m_module.m_mouseSelection.m_started)
		{
			auto logicalPosition = args.Position;
			logicalPosition -= m_module.ScrollOffset;
			m_module.m_mouseSelection.m_endPosition = logicalPosition;

			Point startPoint{
				(std::min)(m_module.m_mouseSelection.m_startPosition.X, m_module.m_mouseSelection.m_endPosition.X),
				(std::min)(m_module.m_mouseSelection.m_startPosition.Y, m_module.m_mouseSelection.m_endPosition.Y)
			};

			Point endPoint{
				(std::max)(m_module.m_mouseSelection.m_startPosition.X, m_module.m_mouseSelection.m_endPosition.X),
				(std::max)(m_module.m_mouseSelection.m_startPosition.Y, m_module.m_mouseSelection.m_endPosition.Y)
			};
			startPoint.Y = (std::max)(startPoint.Y, m_module.m_viewport.BackgroundRect.Y - m_module.ScrollOffset.Y);
			Size boxSize{ (uint32_t)(endPoint.X - startPoint.X), (uint32_t)(endPoint.Y - startPoint.Y) };

			needUpdate |= (boxSize.Width > 0 && boxSize.Height > 0);
			if ((boxSize.Width > 0 && boxSize.Height > 0))
			{
				Rectangle selectionRect{ startPoint.X + m_module.ScrollOffset.X, startPoint.Y + m_module.ScrollOffset.Y * 2 - m_module.m_viewport.BackgroundRect.Y, boxSize.Width, boxSize.Height};

				for (size_t i = 0; i < m_module.List.Items.size(); i++)
				{
					auto& item = m_module.List.Items[i];
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

		m_module.m_hoveredArea = hoveredArea;
		if (needUpdate)
		{
			BT_CORE_TRACE << " -- Listbox Update() " << std::endl;
			Update(graphics);
			GUI::MarkAsUpdated(m_module.m_window);
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
			needUpdate = true;
			m_module.m_mouseSelection.m_started = false;
			m_module.m_mouseSelection.m_selections.clear();
			for (size_t i = 0; i < m_module.List.Items.size(); i++)
			{
				if (m_module.List.Items[i].IsSelected)
				{
					m_module.m_mouseSelection.m_selections.push_back(i);
				}
			}
			GUI::ReleaseCapture(m_module.m_window);
		}
		if (m_module.m_pressedArea == InteractionArea::HeaderSplitter)
		{
			m_module.StopHeadersSizing();
		}

		if (needUpdate)
		{
			Update(graphics);
			GUI::MarkAsUpdated(*m_control);
		}
	}

	void ListBoxReactor::MouseLeave(Graphics& graphics, const ArgMouse& args)
	{
		if (m_module.m_hoveredArea == InteractionArea::HeaderSplitter)
		{
			GUI::ChangeCursor(m_module.m_window, Cursor::Default);
		}

		bool needUpdate = m_module.m_mouseSelection.m_hoveredIndex != -1;
		m_module.m_mouseSelection.m_hoveredIndex = -1;

		m_module.m_hoveredArea = InteractionArea::None;
		if (needUpdate)
		{
			Update(graphics);
			GUI::MarkAsUpdated(m_module.m_window);
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
		auto min= args.IsVertical ? m_module.m_scrollBarVert->GetMin() : m_module.m_scrollBarHoriz->GetMin();
		auto max= args.IsVertical ? m_module.m_scrollBarVert->GetMax() : m_module.m_scrollBarHoriz->GetMax();
		int newOffset = std::clamp((args.IsVertical ? m_module.ScrollOffset.Y : m_module.ScrollOffset.X) + direction, (int)min, (int)max);

		if (args.IsVertical && newOffset != m_module.ScrollOffset.Y ||
			!args.IsVertical && newOffset != m_module.ScrollOffset.X)
		{
			if (args.IsVertical)
			{
				m_module.ScrollOffset.Y = newOffset;
				m_module.CalculateVisibleIndices();
				m_module.m_scrollBarVert->SetValue(newOffset);

				m_module.m_scrollBarVert->Handle()->Renderer.Update();
				GUI::MarkAsUpdated(m_module.m_scrollBarVert->Handle());
			}
			else
			{
				m_module.ScrollOffset.X = newOffset;
				m_module.m_scrollBarHoriz->SetValue(newOffset);

				m_module.m_scrollBarHoriz->Handle()->Renderer.Update();
				GUI::MarkAsUpdated(m_module.m_scrollBarHoriz->Handle());
			}

			Update(graphics);
			GUI::MarkAsUpdated(*m_control);
		}
	}

	void ListBoxReactor::KeyPressed(Graphics& graphics, const ArgKeyboard& args)
	{
		m_module.m_shiftPressed = m_module.m_shiftPressed || args.Key == KeyboardKey::Shift;
		m_module.m_ctrlPressed = m_module.m_ctrlPressed || args.Key == KeyboardKey::Control;

	}

	void ListBoxReactor::KeyReleased(Graphics& graphics, const ArgKeyboard& args)
	{
		if (args.Key == KeyboardKey::Shift) m_module.m_shiftPressed = false;
		if (args.Key == KeyboardKey::Control) m_module.m_ctrlPressed = false;
	}

	void ListBoxReactor::Module::CalculateViewport(ViewportData& viewportData)
	{
		viewportData.BackgroundRect = m_window->Size.ToRectangle();
		viewportData.BackgroundRect.Y = viewportData.BackgroundRect.X = 1;
		viewportData.BackgroundRect.Width -= 2u;
		viewportData.BackgroundRect.Height -= 2u;
		viewportData.InnerMargin = m_window->ToScale(3u);

		auto headerHeight = m_window->ToScale(m_appearance->HeadersHeight);
		viewportData.ItemHeight = m_window->ToScale(m_appearance->ListItemHeight);
		viewportData.ItemHeightWithMargin = viewportData.ItemHeight + viewportData.InnerMargin * 2u;

		viewportData.BackgroundRect.Y += headerHeight;
		viewportData.BackgroundRect.Height -= headerHeight;

		viewportData.ContentSize.Height = (uint32_t)List.Items.size() * (viewportData.ItemHeightWithMargin);

		viewportData.ContentSize.Width = 0u;
		for (size_t i = 0; i < Headers.Items.size(); i++)
		{
			auto headerWidth = m_window->ToScale(Headers.Items[i].Bounds.Width);
			viewportData.ContentSize.Width += headerWidth;
		}

		auto scrollSize = m_window->ToScale(m_window->Appearance->ScrollBarSize);
		viewportData.NeedVerticalScroll = viewportData.ContentSize.Height > viewportData.BackgroundRect.Height;
		if (viewportData.NeedVerticalScroll)
		{
			viewportData.BackgroundRect.Width -= scrollSize;
		}
		viewportData.NeedHorizontalScroll = viewportData.ContentSize.Width > viewportData.BackgroundRect.Width;
		if (viewportData.NeedHorizontalScroll)
		{
			viewportData.BackgroundRect.Height -= scrollSize;
			if (!viewportData.NeedVerticalScroll)
			{
				viewportData.NeedVerticalScroll = viewportData.ContentSize.Height > viewportData.BackgroundRect.Height;
				if (viewportData.NeedVerticalScroll)
				{
					viewportData.BackgroundRect.Width -= scrollSize;
				}
			}
		} 
	}

	void ListBoxReactor::Module::CalculateVisibleIndices()
	{
		if (List.Items.empty() || !m_scrollBarVert)
		{
			m_viewport.StartingVisibleIndex = 0;
			m_viewport.EndingVisibleIndex = List.Items.size();
			return;
		}

		int viewportHeight = (int)(m_viewport.BackgroundRect.Height);
		int itemHeightWithMargin = (int)(m_viewport.ItemHeightWithMargin);
		int startRow = ScrollOffset.Y / itemHeightWithMargin;
		int endRow = 1 + (ScrollOffset.Y + viewportHeight) / itemHeightWithMargin;

		m_viewport.StartingVisibleIndex = startRow;
		m_viewport.EndingVisibleIndex = (std::min)(endRow, (int)List.Items.size());
		BT_CORE_TRACE << "  - starting visible index = " << m_viewport.StartingVisibleIndex << std::endl;
		BT_CORE_TRACE << "  - ending visible index = " << m_viewport.EndingVisibleIndex << std::endl;
	}

	void ListBoxReactor::Module::Erase(size_t index)
	{
		if (List.Items.size() <= index)
		{
			return;
		}

		bool wasSelected = m_mouseSelection.IsSelected(index);
		if (wasSelected)
		{
			m_mouseSelection.Deselect(index);
			if (m_mouseSelection.m_selectedIndex == index || m_mouseSelection.m_selectedIndex >= List.Items.size())
			{
				m_mouseSelection.m_selectedIndex = -1;
			}
		}
		for (size_t i = 0; i < m_mouseSelection.m_selections.size(); i++)
		{
			auto& itemIndex = m_mouseSelection.m_selections[i];
			if (itemIndex > index)
				--itemIndex;
		}
		auto it = List.Items.begin() + index;
		List.Items.erase(it);

		CalculateViewport(m_viewport);
		CalculateVisibleIndices();
		UpdateScrollBars();

		if (index < List.Items.size())
		{
			BuildListItemBounds(index);
		}

		if (m_viewport.NeedVerticalScroll)
		{
			m_scrollBarVert->Handle()->Renderer.Update();
			//GUI::RefreshWindow(m_scrollBarVert->Handle());
		}
		if (m_viewport.NeedHorizontalScroll)
		{
			m_scrollBarHoriz->Handle()->Renderer.Update();
			//GUI::RefreshWindow(m_scrollBarHoriz->Handle());
		}

		m_window->Renderer.Update();
		GUI::RefreshWindow(m_window);
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
			offset.X = Headers.Items[startIndex - 1].Bounds.X + (int)Headers.Items[startIndex - 1].Bounds.Width;
		}
		for (size_t i = startIndex; i < Headers.Items.size(); i++)
		{
			Headers.Items[i].Bounds.X = offset.X;

			offset.X += Headers.Items[i].Bounds.Width;
		}
	}

	void ListBoxReactor::Module::BuildListItemBounds(size_t startIndex)
	{
		auto listItemHeight = m_window->ToScale(m_appearance->ListItemHeight);
		auto innerMarginInt = static_cast<int>(m_viewport.InnerMargin);
		Point offset{ 0,innerMarginInt };

		if (startIndex > 0)
		{
			offset.Y = List.Items[startIndex - 1].Bounds.Y + (int)List.Items[startIndex - 1].Bounds.Height + innerMarginInt;
		}

		for (size_t i = startIndex; i < List.Items.size(); i++)
		{
			List.Items[i].Bounds.X = offset.X;
			List.Items[i].Bounds.Y = offset.Y;
			List.Items[i].Bounds.Height = listItemHeight;
			List.Items[i].Bounds.Width = m_viewport.ContentSize.Width;

			offset.Y += (int)List.Items[i].Bounds.Height + innerMarginInt * 2;
		}
	}

	void ListBoxReactor::DrawStringInBox(Graphics& graphics, const std::string& str, const Rectangle& boxBounds)
	{
		auto textExtent = graphics.GetTextExtent(str);
		if (boxBounds.X + (int)textExtent.Width < 0 /* || boxBounds.X >= */)
		{
			return;
		}

		if (textExtent.Width < boxBounds.Width)
		{
			graphics.DrawString({ boxBounds.X, boxBounds.Y + ((int)(boxBounds.Height - textExtent.Height) >> 1) }, str, m_module.m_appearance->Foreground);
			return;
		}

		auto ellipsisTextExtent = graphics.GetTextExtent("...").Width;
		for (size_t i = str.size(); i >= 1; --i)
		{
			auto subStr = str.substr(0, i);// +"...";
			auto subTextExtent = graphics.GetTextExtent(subStr).Width;
			if (boxBounds.X + (int)(subTextExtent + ellipsisTextExtent) <= (int)boxBounds.Width - 2)
			{
				graphics.DrawString({ boxBounds.X, boxBounds.Y + ((int)(boxBounds.Height - textExtent.Height) >> 1) }, subStr + "...", m_module.m_appearance->Foreground);
				break;
			}
		}
	}

	void ListBoxReactor::Module::AppendHeader(const std::string& text, uint32_t width)
	{
		auto startIndex = Headers.Items.size();
		Headers.Items.emplace_back(text, (std::max)(width, LISTBOX_MIN_HEADER_WIDTH));

		CalculateViewport(m_viewport); 
		CalculateVisibleIndices();
		BuildHeaderBounds(startIndex);
	}

	void ListBoxReactor::Module::Append(const std::string& text)
	{
		auto startIndex = List.Items.size();
		List.Items.emplace_back(text);

		CalculateViewport(m_viewport);
		CalculateVisibleIndices();
		BuildListItemBounds(startIndex);
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

	void ListBox::Append(const std::string& text)
	{
		m_reactor.GetModule().Append(text);
	}

	void ListBox::Append(std::initializer_list<std::string> texts)
	{
		m_reactor.GetModule().Append(texts);
	}

	void ListBox::Clear()
	{
		m_reactor.GetModule().Clear();
	}

	void ListBox::ClearHeaders()
	{
	}

	void ListBox::Erase(uint32_t index)
	{
		m_reactor.GetModule().Erase(index);
	}

	void ListBox::EnableMultiselection(bool enabled)
	{
		m_reactor.GetModule().EnableMultiselection(enabled);
	}

	std::vector<size_t> ListBox::GetSelected() const
	{
		return m_reactor.GetModule().GetSelectedItems();
	}

	void ListBoxReactor::Module::Append(std::initializer_list<std::string> texts)
	{
		auto startIndex = List.Items.size();

		auto headersCount = Headers.Items.size();
		auto& item = List.Items.emplace_back("{}");
		size_t position = 0;
		for (auto& text : texts)
		{
			if (item.Cells.size() == position)
			{
				item.Cells.emplace_back(text);
			}
			else
			{
				item.Cells[position] = text;
			}
			++position;
			if (position >= headersCount)
				break;
		}

		CalculateViewport(m_viewport);
		CalculateVisibleIndices();
		BuildListItemBounds(startIndex);
	}

	void ListBoxReactor::Module::Clear()
	{
		bool needUpdate = !List.Items.empty();
		List.Items.clear();

		CalculateViewport(m_viewport);
		CalculateVisibleIndices();
		UpdateScrollBars();

		if (m_viewport.NeedHorizontalScroll)
		{
			m_scrollBarHoriz->Handle()->Renderer.Update();
		}
		if (needUpdate)
		{
			m_window->Renderer.Update();
			GUI::RefreshWindow(m_window);
		}
	}

	bool ListBoxReactor::Module::UpdateScrollBars()
	{
		auto scrollSize = m_window->ToScale(m_window->Appearance->ScrollBarSize);
		bool needUpdate = false;

		if (m_viewport.NeedVerticalScroll)
		{
			auto listItemHeight = m_window->ToScale(m_appearance->ListItemHeight) + m_viewport.InnerMargin * 2u;
			Rectangle scrollRect{ static_cast<int>(m_window->Size.Width - scrollSize) - 1, 1, scrollSize, m_window->Size.Height - 2u };
			if (m_viewport.NeedHorizontalScroll)
			{
				scrollRect.Height -= scrollSize;
			}

			if (!m_scrollBarVert)
			{
				m_scrollBarVert = std::make_unique<ScrollBar>(m_window, false, scrollRect);
				m_scrollBarVert->GetEvents().ValueChanged.Connect([this](const ArgScrollBar& args)
					{
						ScrollOffset.Y = args.Value;
						CalculateVisibleIndices();

						m_window->Renderer.Update();
						GUI::RefreshWindow(m_window);
					});
			}
			else
			{
				GUI::MoveWindow(m_scrollBarVert->Handle(), scrollRect);
			}

			m_scrollBarVert->SetMinMax(0, (int)(m_viewport.ContentSize.Height - m_viewport.BackgroundRect.Height));
			m_scrollBarVert->SetPageStepValue(m_viewport.BackgroundRect.Height);
			m_scrollBarVert->SetStepValue(listItemHeight);

			ScrollOffset.Y = m_scrollBarVert->GetValue();
			CalculateVisibleIndices();
			needUpdate = true;
		}
		else if (!m_viewport.NeedVerticalScroll && m_scrollBarVert)
		{
			m_scrollBarVert.reset();
			ScrollOffset.Y = 0;
			CalculateVisibleIndices();

			needUpdate = true;
		}

		if (m_viewport.NeedHorizontalScroll)
		{
			Rectangle scrollRect{ 1, static_cast<int>(m_window->Size.Height - scrollSize) - 1, m_window->Size.Width - 2u, scrollSize };
			if (m_viewport.NeedVerticalScroll)
			{
				scrollRect.Width -= scrollSize;
			}

			if (!m_scrollBarHoriz)
			{
				m_scrollBarHoriz = std::make_unique<ScrollBar>(m_window, false, scrollRect, false);
				m_scrollBarHoriz->GetEvents().ValueChanged.Connect([this](const ArgScrollBar& args)
					{
						ScrollOffset.X = args.Value;

						m_window->Renderer.Update();
						GUI::RefreshWindow(m_window);
					});
			}
			else
			{
				GUI::MoveWindow(m_scrollBarHoriz->Handle(), scrollRect);
			}

			m_scrollBarHoriz->SetMinMax(0, (int)(m_viewport.ContentSize.Width - m_viewport.BackgroundRect.Width));
			m_scrollBarHoriz->SetPageStepValue(m_viewport.BackgroundRect.Width);
			m_scrollBarHoriz->SetStepValue(scrollSize);

			ScrollOffset.X = m_scrollBarHoriz->GetValue();
			needUpdate = true;
		}
		else if (!m_viewport.NeedHorizontalScroll && m_scrollBarHoriz)
		{
			m_scrollBarHoriz.reset();
			ScrollOffset.X = 0;

			needUpdate = true;
		}

		return needUpdate;
	}

	void ListBoxReactor::DrawHeaders(Graphics& graphics)
	{
		auto headerHeight = m_module.m_window->ToScale(m_module.m_appearance->HeadersHeight);
		auto leftMarginTextHeader = m_module.m_window->ToScale(5u);
		graphics.DrawRectangle({ 0,0, m_module.m_window->Size.Width, headerHeight }, m_module.m_appearance->ButtonBackground, true);
		graphics.DrawLine({ m_module.m_viewport.BackgroundRect.X, (int)headerHeight - 1 }, { (int)m_module.m_window->Size.Width - 1, (int)headerHeight - 1 }, m_module.m_appearance->BoxPressedBackground);
		graphics.DrawLine({ m_module.m_viewport.BackgroundRect.X, (int)headerHeight }, { (int)m_module.m_window->Size.Width - 1, (int)headerHeight }, m_module.m_appearance->Foreground);
		Point headerOffset{ -m_module.ScrollOffset.X, 0 };

		for (size_t i = 0; i < m_module.Headers.Items.size(); i++)
		{
			const auto& header = m_module.Headers.Items[i];
			auto headerWidth = m_module.m_window->ToScale(header.Bounds.Width);
			auto headerWidthInt = (int)headerWidth;
			if (headerOffset.X + headerWidthInt < 0 || headerOffset.X >= (int)m_module.m_viewport.BackgroundRect.Width)
			{
				headerOffset.X += headerWidthInt;
				continue;
			}

			DrawStringInBox(graphics, header.Name, { headerOffset.X + (int)leftMarginTextHeader, 0, headerWidth - leftMarginTextHeader, headerHeight });

			graphics.DrawLine({ headerOffset.X + headerWidthInt - 1, 0 }, { headerOffset.X + headerWidthInt - 1, (int)headerHeight - 1 }, m_module.m_appearance->BoxPressedBackground);
			graphics.DrawLine({ headerOffset.X + headerWidthInt, 0 }, { headerOffset.X + headerWidthInt, (int)headerHeight - 1 }, m_module.m_appearance->Foreground);

			headerOffset.X += headerWidthInt;
		}
	}

	void ListBoxReactor::DrawList(Graphics& graphics)
	{
		auto headerHeight = m_module.m_window->ToScale(m_module.m_appearance->HeadersHeight);

		Point listOffset{ -m_module.ScrollOffset.X, m_module.m_viewport.BackgroundRect.Y - m_module.ScrollOffset.Y };
		auto itemHeight = m_module.m_viewport.ItemHeight;
		auto itemHeightWithMargin = m_module.m_viewport.ItemHeightWithMargin;
		auto leftMarginListItemText = m_module.m_window->ToScale(3u);

		for (size_t i = m_module.m_viewport.StartingVisibleIndex; i < m_module.m_viewport.EndingVisibleIndex; i++)
		{
			const auto& item = m_module.List.Items[i];
			listOffset.X = -m_module.ScrollOffset.X;

			bool isHovered = m_module.m_mouseSelection.m_hoveredIndex == (int)i;
			bool isSelected = item.IsSelected;
			if (isHovered || isSelected)
			{
				auto color = isSelected ? m_module.m_appearance->HighlightColor : m_module.m_appearance->ItemCollectionHightlightBackground;
				graphics.DrawRectangle({ listOffset.X, listOffset.Y + (int)m_module.m_viewport.InnerMargin + (int)(itemHeightWithMargin * i), m_module.m_viewport.ContentSize.Width, itemHeight }, color, true);
			}

			for (size_t j = 0; j < item.Cells.size(); j++)
			{
				const auto& cell = item.Cells[j];
				const auto& header = m_module.Headers.Items[j];
				auto headerWidth = m_module.m_window->ToScale(header.Bounds.Width);
				auto headerWidthInt = static_cast<int>(headerWidth);
				if (listOffset.X + headerWidthInt < 0)
				{
					listOffset.X += headerWidthInt;
					continue;
				}
				else if (listOffset.X >= (int)m_module.m_viewport.BackgroundRect.Width)
				{
					break;
				}

				DrawStringInBox(graphics, cell.Text, { listOffset.X + (int)leftMarginListItemText, listOffset.Y + (int)m_module.m_viewport.InnerMargin + (int)(itemHeightWithMargin * i), headerWidth, itemHeight });

				listOffset.X += headerWidthInt;
			}
			//listOffset.Y += (int)(itemHeight);
		}
	}

	bool ListBoxReactor::Module::HandleMultiSelection(int itemIndexAtPosition, const ArgMouse& args)
	{
		bool needUpdate = false;

		if (!m_ctrlPressed && !m_shiftPressed)
		{
			if (!args.ButtonState.RightButton || !List.Items[itemIndexAtPosition].IsSelected)
			{
				ClearSelection();
			}
			if (!List.Items[itemIndexAtPosition].IsSelected)
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

	void ListBoxReactor::Module::ToggleItemSelection(int itemIndexAtPosition)
	{
		auto& item = List.Items[itemIndexAtPosition];
		item.IsSelected = !item.IsSelected;

		if (item.IsSelected)
		{
			m_mouseSelection.m_selections.push_back(itemIndexAtPosition);
		}
		else
		{
			auto it = std::remove(m_mouseSelection.m_selections.begin(), m_mouseSelection.m_selections.end(), itemIndexAtPosition);
			m_mouseSelection.m_selections.erase(it, m_mouseSelection.m_selections.end());
		}
	}

	void ListBoxReactor::Module::StartSelectionRectangle(const Point& mousePosition)
	{
		auto logicalPosition = mousePosition;
		logicalPosition -= ScrollOffset;
		m_mouseSelection.m_started = true;
		m_mouseSelection.m_startPosition = logicalPosition;
		m_mouseSelection.m_endPosition = logicalPosition;

		m_mouseSelection.m_inverseSelection = (m_ctrlPressed && !m_shiftPressed);

		m_mouseSelection.m_selections.clear();
		m_mouseSelection.m_alreadySelected.clear();

		for (size_t i = 0; i < List.Items.size(); i++)
		{
			if (List.Items[i].IsSelected)
			{
				m_mouseSelection.m_selections.push_back(i);
				m_mouseSelection.m_alreadySelected.push_back(i);
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
				for (const auto& index : m_mouseSelection.m_selections)
				{
					List.Items[index].IsSelected = false;
				}
				m_mouseSelection.m_selections.clear();
				m_mouseSelection.m_alreadySelected.clear();

				m_mouseSelection.m_selectedIndex = -1;
				return true;
			}
		}
		return false;
	}

	bool ListBoxReactor::Module::ClearSingleSelection()
	{
		if (m_mouseSelection.m_selectedIndex != -1)
		{
			List.Items[m_mouseSelection.m_selectedIndex].IsSelected = false;
			m_mouseSelection.m_selections.clear();
			m_mouseSelection.m_selectedIndex = -1;
			return true;
		}
		return false;
	}

	std::vector<size_t> ListBoxReactor::Module::GetSelectedItems() const
	{
		return m_mouseSelection.m_selections;
	}

	void ListBoxReactor::Module::ClearSelection()
	{
		for (size_t i = 0; i < m_mouseSelection.m_selections.size(); i++)
		{
			List.Items[m_mouseSelection.m_selections[i]].IsSelected = false;
		}
		m_mouseSelection.m_selections.clear();
	}

	void ListBoxReactor::Module::SelectItem(int index)
	{
		List.Items[index].IsSelected = true;
		m_mouseSelection.m_selections.push_back(index);
		m_mouseSelection.m_selectedIndex = index;
		EnsureVisibility(index);
	}

	void ListBoxReactor::Module::EnsureVisibility(int lastSelectedIndex)
	{
		if (!m_scrollBarVert)
		{
			return;
		}


		//Rectangle itemBounds{0, lastSelectedIndex * (int)m_viewport.ItemHeightWithMargin ,m_viewport.ContentSize.Width,  m_viewport.ItemHeightWithMargin };
		//itemBounds.Y -= ScrollOffset.Y;

		//if (m_viewport.BackgroundRect.Contains(itemBounds))
		//{
		//	return;
		//}

		///*auto boundsY = lastSelectedIndex * (int)m_viewport.ItemHeightWithMargin - ScrollOffset.Y;
		//if (boundsY >= m_viewport.BackgroundRect.Y && boundsY <= (int)m_viewport.BackgroundRect.Height)
		//{
		//	return;
		//}*/

		//m_state.m_offset = std::clamp(m_state.m_offset + offsetAdjustment, m_scrollBar->GetMin(), m_scrollBar->GetMax());
		//CalculateVisibleIndices();

		//m_scrollBarVert->SetValue(ScrollOffset.Y);

		//m_scrollBarVert->Handle()->Renderer.Update();
		//GUI::RefreshWindow(m_scrollBarVert->Handle());
	}

	void ListBoxReactor::Module::PerformRangeSelection(int itemIndexAtPosition)
	{
		int minIndex = (std::min)(m_mouseSelection.m_selectedIndex, itemIndexAtPosition);
		int maxIndex = (std::max)(m_mouseSelection.m_selectedIndex, itemIndexAtPosition);

		for (int i = minIndex; i <= maxIndex; ++i)
		{
			List.Items[i].IsSelected = true;
			if (std::find(m_mouseSelection.m_selections.begin(), m_mouseSelection.m_selections.end(), i) == m_mouseSelection.m_selections.end())
			{
				m_mouseSelection.m_selections.push_back(i);
			}
		}
	}

	Berta::ListBoxReactor::InteractionArea ListBoxReactor::Module::DetermineHoverArea(const Point& mousePosition)
	{
		auto headerHeight = m_window->ToScale(m_appearance->HeadersHeight);
		if (!m_window->Size.IsInside(mousePosition))
		{
			return InteractionArea::None;
		}

		if (mousePosition.Y <= (int)headerHeight)
		{
			Point headerOffset{ -ScrollOffset.X, 0 };

			auto splitterThreshold = m_window->ToScale(3);
			for (size_t i = 0; i < Headers.Items.size(); i++)
			{
				const auto& header = Headers.Items[i];
				auto headerWidth = m_window->ToScale(header.Bounds.Width);
				auto headerWidthInt = (int)headerWidth;
				if (headerOffset.X + headerWidthInt < -splitterThreshold || headerOffset.X - splitterThreshold >= (int)m_viewport.BackgroundRect.Width)
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

		if (m_viewport.NeedVerticalScroll && m_viewport.NeedHorizontalScroll && 
			mousePosition.X >= (int)(m_viewport.BackgroundRect.Width) &&
			mousePosition.Y >= (int)(m_viewport.BackgroundRect.Height))
		{
			return InteractionArea::None;
		}

		auto itemHeight = m_window->ToScale(m_appearance->ListItemHeight) + m_viewport.InnerMargin * 2u;
		auto itemHeightInt = static_cast<int>(itemHeight);

		auto positionX = mousePosition.X - m_viewport.BackgroundRect.X + ScrollOffset.X;
		auto positionY = mousePosition.Y - m_viewport.BackgroundRect.Y + ScrollOffset.Y;
		int index = positionY / itemHeightInt;

		if (index < List.Items.size())
		{
			if (m_viewport.InnerMargin)
			{
				auto topBound = index * itemHeightInt;
				auto bottomBound = topBound + itemHeightInt;
				if ((positionY >= topBound && positionY <= topBound + (int)m_viewport.InnerMargin) ||
					(positionY >= bottomBound - (int)m_viewport.InnerMargin && positionY <= bottomBound) || positionX > (int)m_viewport.ContentSize.Width)
				{
					return InteractionArea::ListBlank;
				}
			}

			return InteractionArea::List;
		}

		return InteractionArea::ListBlank;
	}

	bool ListBoxReactor::MouseSelection::IsSelected(size_t index) const
	{
		return std::find(m_selections.begin(), m_selections.end(), index) != m_selections.end();
	}

	void ListBoxReactor::MouseSelection::Deselect(size_t index)
	{
		auto it = std::find(m_selections.begin(), m_selections.end(), index);
		if (it != m_selections.end())
		{
			m_selections.erase(it);
		}
	}
	int ListBoxReactor::Module::GetHeaderAtMousePosition(const Point& mousePosition)
	{
		Point headerOffset{ -ScrollOffset.X, 0 };

		auto splitterThreshold = m_window->ToScale(3);
		for (size_t i = 0; i < Headers.Items.size(); i++)
		{
			const auto& header = Headers.Items[i];
			auto headerWidth = m_window->ToScale(header.Bounds.Width);
			auto headerWidthInt = (int)headerWidth;
			if (headerOffset.X + headerWidthInt < -splitterThreshold || headerOffset.X - splitterThreshold >= (int)m_viewport.BackgroundRect.Width)
			{
				headerOffset.X += headerWidthInt;
				continue;
			}

			if (mousePosition.X >= headerOffset.X + headerWidthInt - splitterThreshold &&
				mousePosition.X <= headerOffset.X + headerWidthInt + splitterThreshold)
			{
				//return InteractionArea::HeaderSplitter;
				return i;
			}

			headerOffset.X += headerWidthInt;
		}
		return -1;
	}

	void ListBoxReactor::Module::StartHeadersSizing(const Point& mousePosition)
	{
		GUI::Capture(m_window);
		Headers.SelectedIndex = GetHeaderAtMousePosition(mousePosition);
		Headers.MouseDownOffset = mousePosition.X - (int)m_window->ToScale(Headers.Items[Headers.SelectedIndex].Bounds.X + Headers.Items[Headers.SelectedIndex].Bounds.Width);
	}

	void ListBoxReactor::Module::UpdateHeadersSizing(const Point& mousePosition)
	{
		auto& headerBounds = Headers.Items[Headers.SelectedIndex].Bounds;
		auto newWidth = m_window->ToDownScale(mousePosition.X - Headers.MouseDownOffset - m_window->ToScale(headerBounds.X));

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
		BuildHeaderBounds(Headers.SelectedIndex);
	}

	void ListBoxReactor::Module::StopHeadersSizing()
	{
		GUI::ReleaseCapture(m_window);
		Headers.SelectedIndex = -1;
	}
}
