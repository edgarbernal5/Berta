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

		UpdateScrollBars();
	}

	void ListBoxReactor::MouseDown(Graphics& graphics, const ArgMouse& args)
	{
		m_module.m_pressedArea = m_module.m_hoverArea;
		bool needUpdate = false;
		if (m_module.m_pressedArea == InteractionArea::List)
		{
			m_module.m_mouseSelection.m_pressedIndex = m_module.m_mouseSelection.m_hoveredIndex;
			if (m_module.m_mouseSelection.m_selectedIndex != m_module.m_mouseSelection.m_hoveredIndex)
			{
				needUpdate = true;
				m_module.m_mouseSelection.m_selectedIndex = m_module.m_mouseSelection.m_hoveredIndex;
			}
			
		}
		else if (m_module.m_pressedArea == InteractionArea::ListBlank)
		{
			if (m_module.m_mouseSelection.m_selectedIndex != -1)
			{
				needUpdate = true;
				m_module.m_mouseSelection.m_selectedIndex = -1;
			}
			if (m_module.m_multiselection)
			{
				auto logicalPosition = args.Position;
				logicalPosition.Y -= m_module.ScrollOffset.Y;
				m_module.m_mouseSelection.m_started = true;
				m_module.m_mouseSelection.m_startPosition = logicalPosition;
				m_module.m_mouseSelection.m_endPosition = logicalPosition;

				m_module.m_mouseSelection.m_inverseSelection = (m_module.m_ctrlPressed && !m_module.m_shiftPressed);

				GUI::Capture(m_module.m_window);
			}
		}

		if (needUpdate)
		{
			Update(graphics);
			GUI::MarkAsUpdated(m_module.m_window);
		}
	}

	void ListBoxReactor::MouseMove(Graphics& graphics, const ArgMouse& args)
	{
		auto hoveredArea = DetermineHoverArea(args.Position);
		bool needUpdate = hoveredArea != m_module.m_hoverArea;

		if (hoveredArea == InteractionArea::List)
		{
			auto itemHeight = m_module.m_window->ToScale(m_module.m_appearance->ListItemHeight) + m_module.m_viewport.InnerMargin * 2u;

			auto positionY = args.Position.Y - m_module.m_viewport.BackgroundRect.Y + m_module.ScrollOffset.Y;
			m_module.m_mouseSelection.m_hoveredIndex = positionY / (int)itemHeight;
		}
		if (!needUpdate && hoveredArea == InteractionArea::List)
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
		if (hoveredArea == InteractionArea::ListBlank)
		{
			if (m_module.m_mouseSelection.m_hoveredIndex != -1)
			{
				m_module.m_mouseSelection.m_hoveredIndex = -1;
				needUpdate = true;
			}
		}
		m_module.m_hoverArea = hoveredArea;
		if (needUpdate)
		{
			Update(graphics);
			GUI::MarkAsUpdated(m_module.m_window);
		}
	}

	void ListBoxReactor::MouseUp(Graphics& graphics, const ArgMouse& args)
	{
		bool needUpdate = false;

		if (m_module.m_mouseSelection.m_started)
		{
			m_module.m_mouseSelection.m_started = false;
			needUpdate = true;
			
			GUI::ReleaseCapture(m_module.m_window);
		}

		if (needUpdate)
		{
			Update(graphics);
			GUI::MarkAsUpdated(*m_control);
		}
	}

	void ListBoxReactor::MouseLeave(Graphics& graphics, const ArgMouse& args)
	{
		m_module.m_hoverArea = InteractionArea::None;
		m_module.m_mouseSelection.m_hoveredIndex = -1;

		Update(graphics);
		GUI::MarkAsUpdated(m_module.m_window);
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
		auto itemHeight = m_window->ToScale(m_appearance->ListItemHeight);

		viewportData.BackgroundRect.Y += headerHeight;
		viewportData.BackgroundRect.Height -= headerHeight;

		viewportData.ContentSize.Height = (uint32_t)List.Items.size() * (itemHeight + viewportData.InnerMargin * 2u);

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

	void ListBoxReactor::Module::EnableMultiselection(bool enabled)
	{
		m_multiselection = enabled;
	}

	void ListBoxReactor::Module::BuildHeaderBounds(uint32_t startIndex)
	{
		Point offset{ 0,0 };
		if (startIndex > 0)
		{
			offset.X = Headers.Items[startIndex - 1].Bounds.X + Headers.Items[startIndex - 1].Bounds.Width;
		}
		for (size_t i = startIndex; i < Headers.Items.size(); i++)
		{
			Headers.Items[i].Bounds.X = offset.X;

			offset.X += Headers.Items[i].Bounds.Width;
		}
	}

	void ListBoxReactor::Module::BuildListItemBounds(uint32_t startIndex)
	{
		auto listItemHeight = m_window->ToScale(m_appearance->ListItemHeight);
		Point offset{ 0,0 };
		if (startIndex > 0)
		{
			offset.Y = List.Items[startIndex - 1].Bounds.Y + List.Items[startIndex - 1].Bounds.Height;
		}
		for (size_t i = startIndex; i < List.Items.size(); i++)
		{
			List.Items[i].Bounds.Y = offset.Y;
			List.Items[i].Bounds.Height = listItemHeight;

			offset.Y += List.Items[i].Bounds.Height;
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
		auto startIndex = static_cast<uint32_t>(Headers.Items.size());
		Headers.Items.emplace_back(text, (std::max)(width, LISTBOX_MIN_HEADER_WIDTH));

		BuildHeaderBounds(startIndex);
		CalculateViewport(m_viewport);
	}

	void ListBoxReactor::Module::Append(const std::string& text)
	{
		List.Items.emplace_back(text);
		CalculateViewport(m_viewport);
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

	void ListBox::EnableMultiselection(bool enabled)
	{
		m_reactor.GetModule().EnableMultiselection(enabled);
	}

	void ListBoxReactor::Module::Append(std::initializer_list<std::string> texts)
	{
		auto startIndex = static_cast<uint32_t>(List.Items.size());

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

		BuildListItemBounds(startIndex);
		CalculateViewport(m_viewport);
	}

	void ListBoxReactor::Module::Clear()
	{

	}

	bool ListBoxReactor::UpdateScrollBars()
	{
		auto scrollSize = m_module.m_window->ToScale(m_module.m_window->Appearance->ScrollBarSize);
		bool needUpdate = false;
		if (m_module.m_viewport.NeedVerticalScroll)
		{
			auto listItemHeight = m_module.m_window->ToScale(m_module.m_appearance->ListItemHeight);
			Rectangle scrollRect{ static_cast<int>(m_module.m_window->Size.Width - scrollSize) - 1, 1, scrollSize, m_module.m_window->Size.Height - 2u };
			if (m_module.m_viewport.NeedHorizontalScroll)
			{
				scrollRect.Height -= scrollSize;
			}

			if (!m_module.m_scrollBarVert)
			{
				m_module.m_scrollBarVert = std::make_unique<ScrollBar>(m_module.m_window, false, scrollRect);
				m_module.m_scrollBarVert->GetEvents().ValueChanged.Connect([this](const ArgScrollBar& args)
					{
						m_module.ScrollOffset.Y = args.Value;

						m_module.m_window->Renderer.Update();
						GUI::RefreshWindow(m_module.m_window);
					});
			}
			else
			{
				GUI::MoveWindow(m_module.m_scrollBarVert->Handle(), scrollRect);
			}

			m_module.m_scrollBarVert->SetMinMax(0, (int)(m_module.m_viewport.ContentSize.Height - m_module.m_viewport.BackgroundRect.Height));
			m_module.m_scrollBarVert->SetPageStepValue(m_module.m_viewport.BackgroundRect.Height);
			m_module.m_scrollBarVert->SetStepValue(listItemHeight);
			needUpdate = true;
		}
		else if (!m_module.m_viewport.NeedVerticalScroll && m_module.m_scrollBarVert)
		{
			m_module.m_scrollBarVert.reset();
			m_module.ScrollOffset.Y = 0;

			needUpdate = true;
		}

		if (m_module.m_viewport.NeedHorizontalScroll)
		{
			Rectangle scrollRect{ 1, static_cast<int>(m_module.m_window->Size.Height - scrollSize) - 1, m_module.m_window->Size.Width - 2u, scrollSize };
			if (m_module.m_viewport.NeedVerticalScroll)
			{
				scrollRect.Width -= scrollSize;
			}

			if (!m_module.m_scrollBarHoriz)
			{
				m_module.m_scrollBarHoriz = std::make_unique<ScrollBar>(m_module.m_window, false, scrollRect, false);
				m_module.m_scrollBarHoriz->GetEvents().ValueChanged.Connect([this](const ArgScrollBar& args)
					{
						m_module.ScrollOffset.X = args.Value;

						m_module.m_window->Renderer.Update();
						GUI::RefreshWindow(m_module.m_window);
					});
			}
			else
			{
				GUI::MoveWindow(m_module.m_scrollBarHoriz->Handle(), scrollRect);
			}

			m_module.m_scrollBarHoriz->SetMinMax(0, (int)(m_module.m_viewport.ContentSize.Width - m_module.m_viewport.BackgroundRect.Width));
			m_module.m_scrollBarHoriz->SetPageStepValue(m_module.m_viewport.BackgroundRect.Width);
			m_module.m_scrollBarHoriz->SetStepValue(scrollSize);
			needUpdate = true;
		}
		else if (!m_module.m_viewport.NeedHorizontalScroll && m_module.m_scrollBarHoriz)
		{
			m_module.m_scrollBarHoriz.reset();
			m_module.ScrollOffset.X = 0;

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

			DrawStringInBox(graphics, header.Name, { headerOffset.X + (int)leftMarginTextHeader, 0, headerWidth, headerHeight });

			graphics.DrawLine({ headerOffset.X + headerWidthInt - 1, 0 }, { headerOffset.X + headerWidthInt - 1, (int)headerHeight - 1 }, m_module.m_appearance->BoxPressedBackground);
			graphics.DrawLine({ headerOffset.X + headerWidthInt, 0 }, { headerOffset.X + headerWidthInt, (int)headerHeight - 1 }, m_module.m_appearance->Foreground);

			headerOffset.X += headerWidthInt;
		}
	}

	void ListBoxReactor::DrawList(Graphics& graphics)
	{
		auto headerHeight = m_module.m_window->ToScale(m_module.m_appearance->HeadersHeight);

		Point listOffset{ -m_module.ScrollOffset.X, m_module.m_viewport.BackgroundRect.Y - m_module.ScrollOffset.Y };
		auto itemHeight = m_module.m_window->ToScale(m_module.m_appearance->ListItemHeight);
		auto leftMarginListItemText = m_module.m_window->ToScale(3u);

		for (size_t i = 0; i < m_module.List.Items.size(); i++)
		{
			const auto& item = m_module.List.Items[i];
			listOffset.X = -m_module.ScrollOffset.X;

			bool isHovered = m_module.m_mouseSelection.m_hoveredIndex == (int)i;
			bool isSelected = m_module.m_mouseSelection.m_selectedIndex == (int)i;
			if (isHovered || isSelected)
			{
				auto color = isSelected ? m_module.m_appearance->HighlightColor : m_module.m_appearance->ItemCollectionHightlightBackground;
				graphics.DrawRectangle({ listOffset.X, listOffset.Y + (int)m_module.m_viewport.InnerMargin, m_module.m_viewport.ContentSize.Width, itemHeight }, color, true);
			}
			for (size_t j = 0; j < item.Cells.size(); j++)
			{
				const auto& cell = item.Cells[j];
				const auto& header = m_module.Headers.Items[j];
				auto headerWidth = m_module.m_window->ToScale(header.Bounds.Width);
				auto headerWidthInt = (int)headerWidth;
				if (listOffset.X + headerWidthInt < 0)
				{
					listOffset.X += headerWidthInt;
					continue;
				}
				else if (listOffset.X >= (int)m_module.m_viewport.BackgroundRect.Width)
				{
					break;
				}

				DrawStringInBox(graphics, cell.Text, { listOffset.X + (int)leftMarginListItemText, listOffset.Y + (int)m_module.m_viewport.InnerMargin, headerWidth, itemHeight });

				listOffset.X += headerWidthInt;
			}
			listOffset.Y += (int)(itemHeight + m_module.m_viewport.InnerMargin * 2u);
		}
	}

	Berta::ListBoxReactor::InteractionArea ListBoxReactor::DetermineHoverArea(const Point& mousePosition)
	{
		auto headerHeight = m_module.m_window->ToScale(m_module.m_appearance->HeadersHeight);
		if (mousePosition.Y <= (int)headerHeight)
		{
			Point headerOffset{ -m_module.ScrollOffset.X, 0 };

			auto splitterThreshold = m_module.m_window->ToScale(3);
			for (size_t i = 0; i < m_module.Headers.Items.size(); i++)
			{
				const auto& header = m_module.Headers.Items[i];
				auto headerWidth = m_module.m_window->ToScale(header.Bounds.Width);
				auto headerWidthInt = (int)headerWidth;
				if (headerOffset.X + headerWidthInt < -splitterThreshold || headerOffset.X - splitterThreshold >= (int)m_module.m_viewport.BackgroundRect.Width)
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

		auto itemHeight = m_module.m_window->ToScale(m_module.m_appearance->ListItemHeight) + m_module.m_viewport.InnerMargin * 2u;
		auto itemHeightInt = static_cast<int>(itemHeight);

		auto positionY = mousePosition.Y - m_module.m_viewport.BackgroundRect.Y + m_module.ScrollOffset.Y;
		int index = positionY / itemHeightInt;

		if (index < m_module.List.Items.size())
		{
			if (m_module.m_viewport.InnerMargin)
			{
				auto topBound = index * itemHeightInt;
				auto bottomBound = topBound + itemHeightInt;
				if ((positionY >= topBound && positionY <= topBound + (int)m_module.m_viewport.InnerMargin) ||
					(positionY >= bottomBound - (int)m_module.m_viewport.InnerMargin && positionY <= topBound))
				{
					return InteractionArea::ListBlank;
				}
			}
			return InteractionArea::List;
		}

		return InteractionArea::None;
	}
}
